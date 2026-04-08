#include <genesis.h>

#include "fm_cursor.h"
#include "resources.h"

/** FB 数値列の `fm_display_draw` における X オフセット（タイル）。 */
#define FM_GLOBAL_FB_X_OFF 4u

static Sprite *s_cursor;

static struct
{
    bool on_op;
    u8 g_slot;
    u8 op_row;
    u8 op_col;
    u8 last_g_slot;
    u8 last_op_col;
} s_cur;

/** 各 OP 列のカーソル用桁数（スプライト幅）。末尾は SSG の 100 の位（1 桁）と下 2 桁。 */
static const u8 OP_MAX_DIGITS[FM_OP_NUM_COLS] = {
    2u, 2u, 2u, 2u, 2u, 3u, 1u, 2u, 1u, 1u, 2u,
};

static u8 cursor_max_digits(void)
{
    if (!s_cur.on_op)
        return 1u;
    return OP_MAX_DIGITS[s_cur.op_col];
}

static const SpriteDefinition *sprite_def_for_digits(u8 digits)
{
    if (digits >= 3u)
        return &spr_cursor_3d;
    if (digits == 2u)
        return &spr_cursor_2d;
    return &spr_cursor_1d;
}

static u16 cursor_left_tile_x(void)
{
    const u16 bx = FM_DISPLAY_OFFSET_X;

    if (!s_cur.on_op)
    {
        if (s_cur.g_slot == 0u)
        {
            const u8 w = FM_VAL_WIDTH_ALG;
            const u8 md = 1u;
            return bx + (u16)(w - md);
        }
        const u8 w = FM_VAL_WIDTH_FB;
        const u8 md = 1u;
        return bx + FM_GLOBAL_FB_X_OFF + (u16)(w - md);
    }

    const u8 w = fm_op_val_width[s_cur.op_col];
    const u8 md = OP_MAX_DIGITS[s_cur.op_col];
    return bx + fm_op_col_x[s_cur.op_col] + (u16)(w - md);
}

static u16 cursor_value_row_tile_y(void)
{
    if (!s_cur.on_op)
        return FM_DISPLAY_OFFSET_Y + 2u;
    return FM_DISPLAY_OFFSET_Y + 4u + (u16)s_cur.op_row;
}

/** OP 各列の最大値（`docs/display_layout.md` / YM2612）。 */
static const u8 OP_PARAM_MAX[FM_OP_NUM_COLS] = {
    31u, 31u, 31u, 15u, 15u, 127u, 3u, 15u, 7u, 1u, 15u,
};

static void u8_add_clamp(u8 *v, int delta, u8 maxv)
{
    int n = (int)*v + delta;
    if (n < 0)
        n = 0;
    else if (n > (int)maxv)
        n = (int)maxv;
    *v = (u8)n;
}

/** @return 値が変化したとき真 */
static bool patch_adjust_at_cursor(FmPatch *patch, int delta)
{
    if (delta == 0)
        return FALSE;

    if (!s_cur.on_op)
    {
        if (s_cur.g_slot == 0u)
        {
            const u8 prev = patch->alg;
            u8_add_clamp(&patch->alg, delta, 7u);
            return patch->alg != prev;
        }
        const u8 prev = patch->fb;
        u8_add_clamp(&patch->fb, delta, 7u);
        return patch->fb != prev;
    }

    FmOpParams *op = &patch->op[s_cur.op_row];
    u8 *cell = &op->raw[s_cur.op_col];
    const u8 prev = *cell;
    u8_add_clamp(cell, delta, OP_PARAM_MAX[s_cur.op_col]);
    return *cell != prev;
}

void fm_cursor_init(void)
{
    s_cur.on_op = FALSE;
    s_cur.g_slot = 0u;
    s_cur.op_row = 0u;
    s_cur.op_col = 0u;
    s_cur.last_g_slot = 0u;
    s_cur.last_op_col = 0u;

    PAL_setPalette(FM_CURSOR_PALETTE, pal_cursor_digit.data, DMA);
    s_cursor = SPR_addSprite(&spr_cursor_1d, 0, 0, TILE_ATTR(FM_CURSOR_PALETTE, FALSE, FALSE, FALSE));
    SPR_setVisibility(s_cursor, VISIBLE);
    fm_cursor_refresh_sprite();
}

void fm_cursor_refresh_sprite(void)
{
    const u8 md = cursor_max_digits();
    SPR_setDefinition(s_cursor, sprite_def_for_digits(md));
    SPR_setPriority(s_cursor, FALSE);
    SPR_setPosition(s_cursor, (s16)(cursor_left_tile_x() * 8u), (s16)(cursor_value_row_tile_y() * 8u));
}

void fm_cursor_refresh_sprite_on_note_row(void)
{
    const u16 bx = FM_DISPLAY_OFFSET_X;
    const u16 tx = bx + FM_NOTE_PREFIX_LEN;
    SPR_setDefinition(s_cursor, sprite_def_for_digits(FM_NOTE_VALUE_TILES));
    SPR_setPriority(s_cursor, FALSE);
    SPR_setPosition(s_cursor, (s16)(tx * 8u), (s16)(FM_DISPLAY_OFFSET_Y * 8u));
}

bool fm_cursor_step(u16 joy, u16 joy_prev, FmPatch *patch)
{
    const u16 pressed = (u16)(joy & (u16)~joy_prev);
    bool dirty = FALSE;

    if ((joy & BUTTON_C) && patch != NULL)
    {
        int delta = 0;
        if (pressed & BUTTON_UP)
            delta = 10;
        else if (pressed & BUTTON_DOWN)
            delta = -10;
        else if (pressed & BUTTON_RIGHT)
            delta = 1;
        else if (pressed & BUTTON_LEFT)
            delta = -1;
        if (delta != 0)
            dirty = patch_adjust_at_cursor(patch, delta);
    }
    else
    {
        if (pressed & BUTTON_LEFT)
        {
            if (!s_cur.on_op)
            {
                if (s_cur.g_slot > 0u)
                    s_cur.g_slot--;
            }
            else if (s_cur.op_col > 0u)
                s_cur.op_col--;
        }
        else if (pressed & BUTTON_RIGHT)
        {
            if (!s_cur.on_op)
            {
                if (s_cur.g_slot < 1u)
                    s_cur.g_slot++;
            }
            else if (s_cur.op_col < (FM_OP_NUM_COLS - 1u))
                s_cur.op_col++;
        }
        else if (pressed & BUTTON_UP)
        {
            if (s_cur.on_op)
            {
                if (s_cur.op_row > 0u)
                    s_cur.op_row--;
                else
                {
                    s_cur.on_op = FALSE;
                    s_cur.g_slot = s_cur.last_g_slot;
                }
            }
        }
        else if (pressed & BUTTON_DOWN)
        {
            if (!s_cur.on_op)
            {
                s_cur.on_op = TRUE;
                s_cur.op_row = 0u;
                s_cur.op_col = s_cur.last_op_col;
            }
            else if (s_cur.op_row < 3u)
                s_cur.op_row++;
        }
    }

    if (!s_cur.on_op)
        s_cur.last_g_slot = s_cur.g_slot;
    else
        s_cur.last_op_col = s_cur.op_col;

    fm_cursor_refresh_sprite();
    return dirty;
}
