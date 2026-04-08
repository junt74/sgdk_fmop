#include <genesis.h>

#include "fm_preview.h"
#include "fm_confirm_note.h"
#include "mdsdrv.h"

/** 試聴は port0 の ch2（0-based）。MDSDRV シーケンスとは競合しうるため試聴中は全リクエストを停止する。 */
#define FM_PREVIEW_YM_CH 2u
#define FM_PREVIEW_MIDI_BASE 12u
#define FM_PREVIEW_SEMI_RANGE 108u

static void mds_fm_port0(u8 reg, u8 val)
{
    MDS_command(MDS_CMD_WRITE_FM_PORT0, ((u16)reg << 8) | (u16)val);
}

static const u16 FM_PREVIEW_PITCH[FM_PREVIEW_SEMI_RANGE] = {
    0x0142, 0x0155, 0x0169, 0x017f, 0x0196, 0x01ae, 0x01c7, 0x01e2, 0x01ff, 0x021d, 0x023e, 0x0260, 0x0284, 0x02aa,
    0x02d3, 0x02fe, 0x032b, 0x035b, 0x038e, 0x03c5, 0x03fe, 0x043b, 0x047b, 0x04bf, 0x0508, 0x0554, 0x05a5, 0x05fb,
    0x0656, 0x06b7, 0x071d, 0x0789, 0x07fc, 0x0c3b, 0x0c7b, 0x0cbf, 0x0d08, 0x0d54, 0x0da5, 0x0dfb, 0x0e56, 0x0eb7,
    0x0f1d, 0x0f89, 0x0ffc, 0x143b, 0x147b, 0x14bf, 0x1508, 0x1554, 0x15a5, 0x15fb, 0x1656, 0x16b7, 0x171d, 0x1789,
    0x17fc, 0x1c3b, 0x1c7b, 0x1cbf, 0x1d08, 0x1d54, 0x1da5, 0x1dfb, 0x1e56, 0x1eb7, 0x1f1d, 0x1f89, 0x1ffc, 0x243b,
    0x247b, 0x24bf, 0x2508, 0x2554, 0x25a5, 0x25fb, 0x2656, 0x26b7, 0x271d, 0x2789, 0x27fc, 0x2c3b, 0x2c7b, 0x2cbf,
    0x2d08, 0x2d54, 0x2da5, 0x2dfb, 0x2e56, 0x2eb7, 0x2f1d, 0x2f89, 0x2ffc, 0x343b, 0x347b, 0x34bf, 0x3508, 0x3554,
    0x35a5, 0x35fb, 0x3656, 0x36b7, 0x371d, 0x3789, 0x37fc, 0x3c3b, 0x3c7b, 0x3cbf,
};

static void mds_write_slot(u8 ch, u8 sl, u8 reg, u8 val)
{
    mds_fm_port0((u8)(reg | (u8)(sl * 4u) | ch), val);
}

static void mds_write_ch(u8 ch, u8 reg, u8 val) { mds_fm_port0((u8)(reg | ch), val); }

static void apply_slot_from_op(u8 ch, u8 sl, const FmOpParams *op)
{
    const u8 r30 = (u8)(((op->dt & 7u) << 4) | (op->ml & 15u));
    mds_write_slot(ch, sl, 0x30, r30);
    mds_write_slot(ch, sl, 0x40, (u8)(op->tl & 0x7Fu));
    mds_write_slot(ch, sl, 0x50, (u8)(((op->k & 3u) << 6) | (op->ar & 31u)));
    mds_write_slot(ch, sl, 0x60, (u8)(op->dr & 31u));
    mds_write_slot(ch, sl, 0x70, (u8)(op->sr & 31u));
    mds_write_slot(ch, sl, 0x80, (u8)(((op->sl & 15u) << 4) | (op->rr & 15u)));
    mds_write_slot(ch, sl, 0x90, (u8)(op->ssg_lo & 15u));
}

static void apply_patch_to_channel(const FmPatch *patch, u8 ch)
{
    static const u8 slot_map[4] = {0u, 2u, 1u, 3u};
    u8 i;

    mds_write_ch(ch, 0xB0, (u8)(((patch->fb & 7u) << 3) | (patch->alg & 7u)));
    mds_write_ch(ch, 0xB4, 0xC0);

    for (i = 0u; i < 4u; i++)
        apply_slot_from_op(ch, slot_map[i], &patch->op[i]);
}

static void apply_midi_pitch(u8 ch, u8 midi)
{
    u16 packed;
    u16 fn;
    u8 blk;

    if (midi < FM_PREVIEW_MIDI_BASE)
        midi = FM_PREVIEW_MIDI_BASE;
    if (midi > FM_PREVIEW_MIDI_BASE + FM_PREVIEW_SEMI_RANGE - 1u)
        midi = FM_PREVIEW_MIDI_BASE + (u8)(FM_PREVIEW_SEMI_RANGE - 1u);

    packed = FM_PREVIEW_PITCH[(u16)(midi - FM_PREVIEW_MIDI_BASE)];
    fn = (u16)(packed & 0x7FFu);
    blk = (u8)((packed >> 11) & 7u);

    /* YM2612 $A4: BLK in bits 5-3, FNUM 10-8 in bits 2-0; update applies on final $A0 write. */
    mds_write_ch(ch, 0xA4, (u8)(((blk & 7u) << 3) | ((fn >> 8) & 7u)));
    mds_write_ch(ch, 0xA0, (u8)(fn & 0xFFu));
}

static void preview_stop_mds_tracks(void)
{
    MDS_request(MDS_BGM, 0);
    MDS_request(MDS_SE1, 0);
    MDS_request(MDS_SE2, 0);
    MDS_request(MDS_SE3, 0);
}

static void key_off_ch(u8 ch) { mds_fm_port0(0x28, (u8)(ch & 3u)); }

static void key_on_ch(u8 ch) { mds_fm_port0(0x28, (u8)(0xF0u | (ch & 3u))); }

static bool preview_patch_differs(const FmPatch *a, const FmPatch *b)
{
    u8 i, j;

    if (a->alg != b->alg || a->fb != b->fb)
        return TRUE;
    for (i = 0u; i < 4u; i++)
    {
        for (j = 0u; j < FM_OP_NUM_COLS; j++)
        {
            if (a->op[i].raw[j] != b->op[i].raw[j])
                return TRUE;
        }
    }
    return FALSE;
}

static bool s_preview_active;
static u8 s_preview_midi;
static FmPatch s_preview_patch;

void fm_preview_init(void)
{
    s_preview_active = FALSE;
    s_preview_patch.alg = 0;
    s_preview_patch.fb = 0;
    {
        u8 i, j;
        for (i = 0u; i < 4u; i++)
            for (j = 0u; j < FM_OP_NUM_COLS; j++)
                s_preview_patch.op[i].raw[j] = 0;
    }
    s_preview_midi = 0;
}

void fm_preview_frame(const FmPatch *patch, u16 joy, u16 joy_prev)
{
    const bool a = (joy & BUTTON_A) != 0;
    const bool a_prev = (joy_prev & BUTTON_A) != 0;

    if (!a)
    {
        if (a_prev)
            key_off_ch((u8)FM_PREVIEW_YM_CH);
        s_preview_active = FALSE;
        return;
    }

    {
        const u8 midi = (u8)(FM_PREVIEW_MIDI_BASE + fm_confirm_note_get());
        const bool edge_on = a && !a_prev;
        const bool patch_changed = preview_patch_differs(patch, &s_preview_patch);
        const bool note_changed = !s_preview_active || (midi != s_preview_midi);
        const bool should_retrigger = edge_on || !s_preview_active || patch_changed;
        const bool need_pitch_only = s_preview_active && !should_retrigger && note_changed;

        if (!should_retrigger && !need_pitch_only)
            return;

        if (should_retrigger)
        {
            preview_stop_mds_tracks();
            if (s_preview_active)
                key_off_ch((u8)FM_PREVIEW_YM_CH);

            apply_patch_to_channel(patch, (u8)FM_PREVIEW_YM_CH);
            apply_midi_pitch((u8)FM_PREVIEW_YM_CH, midi);
            key_on_ch((u8)FM_PREVIEW_YM_CH);
            s_preview_active = TRUE;
        }
        else if (need_pitch_only)
            apply_midi_pitch((u8)FM_PREVIEW_YM_CH, midi);

        s_preview_midi = midi;
        s_preview_patch = *patch;
    }
}
