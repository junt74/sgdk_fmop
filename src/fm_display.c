#include <genesis.h>
#include <string.h>

#include "fm_display.h"
#include "fm_confirm_note.h"

/** パラメータラベル前景色（#444 → #444444、SGDK 9bit 色へ変換）。 */
#define FM_LABEL_TEXT_RGB24 0x444444u
/** 確認ノート行の前景色（#FFF）。 */
#define FM_NOTE_TEXT_RGB24 0xFFFFFFu
/** ノート行の描画幅（短い文字列時に右側をスペースで上書きしてゴストを消す）。 */
#define FM_NOTE_LINE_WIDTH 16u

void fm_display_palette_init(void)
{
    PAL_setPalette(FM_LABEL_PALETTE, palette_grey, CPU);
    PAL_setColor(FM_LABEL_PALETTE_TEXT_CRAM_INDEX, RGB24_TO_VDPCOLOR(FM_LABEL_TEXT_RGB24));
    PAL_setPalette(FM_NOTE_PALETTE, palette_grey, CPU);
    PAL_setColor((u16)(FM_NOTE_PALETTE * 16u + 15u), RGB24_TO_VDPCOLOR(FM_NOTE_TEXT_RGB24));
}

/**
 * OP 数値列の先頭タイル X。SSG は画面上 3 桁で `fm_op_col_x[9]` から描画。
 * `[9]`＝100 の位のカーソル、`[10]`＝10・1 の位（2 タイル）のカーソル左端。
 */
const u16 fm_op_col_x[FM_OP_NUM_COLS] = {
    0u, 3u, 6u, 9u, 12u, 15u, 19u, 21u, 24u, 26u, 27u,
};

/** カーソル／内部用の列幅（SSG は末尾 2 要素が 100 の位・下 2 桁）。 */
const u8 fm_op_val_width[FM_OP_NUM_COLS] = {
    FM_VAL_WIDTH_AR, FM_VAL_WIDTH_DR, FM_VAL_WIDTH_SR, FM_VAL_WIDTH_RR, FM_VAL_WIDTH_SL,
    FM_VAL_WIDTH_TL, FM_VAL_WIDTH_K, FM_VAL_WIDTH_ML, FM_VAL_WIDTH_DT,
    FM_VAL_WIDTH_SSG_HI, FM_VAL_WIDTH_SSG_LO,
};

void fm_display_draw(const FmPatch *patch)
{
    char buf[16];
    char note_line[20];
    u16 x = FM_DISPLAY_OFFSET_X;
    u16 y = FM_DISPLAY_OFFSET_Y;

    VDP_setTextPalette(FM_LABEL_PALETTE);
    fm_confirm_note_format(note_line, sizeof(note_line));
    {
        u16 len = (u16)strlen(note_line);
        while (len < FM_NOTE_LINE_WIDTH)
            note_line[len++] = ' ';
        note_line[len] = '\0';
    }
    {
        const u16 label_basetile = TILE_ATTR(FM_LABEL_PALETTE, VDP_getTextPriority(), FALSE, FALSE);
        const u16 value_basetile = TILE_ATTR(FM_NOTE_PALETTE, VDP_getTextPriority(), FALSE, FALSE);
        const char saved = note_line[FM_NOTE_PREFIX_LEN];
        note_line[FM_NOTE_PREFIX_LEN] = '\0';
        VDP_drawTextEx(VDP_getTextPlane(), note_line, label_basetile, x, y, CPU);
        note_line[FM_NOTE_PREFIX_LEN] = saved;
        VDP_drawTextEx(VDP_getTextPlane(), note_line + FM_NOTE_PREFIX_LEN, value_basetile,
                         x + FM_NOTE_PREFIX_LEN, y, CPU);
    }
    y++;

    VDP_drawText("ALG FB", x, y);
    y++;

    VDP_setTextPalette(PAL0);
    sprintf(buf, "%*u", (int)FM_VAL_WIDTH_ALG, (unsigned)patch->alg);
    VDP_drawText(buf, x, y);
    sprintf(buf, "%*u", (int)FM_VAL_WIDTH_FB, (unsigned)patch->fb);
    VDP_drawText(buf, x + 4u, y);
    y++;

    VDP_setTextPalette(FM_LABEL_PALETTE);
    VDP_drawText("AR DR SR RR SL  TL K MLDT SSG", x, y);
    y++;

    VDP_setTextPalette(PAL0);
    for (u16 row = 0; row < 4; row++)
    {
        const FmOpParams *op = &patch->op[row];
        for (u16 c = 0u; c < 9u; c++)
        {
            sprintf(buf, "%*u", (int)fm_op_val_width[c], (unsigned)op->raw[c]);
            VDP_drawText(buf, x + fm_op_col_x[c], y);
        }
        {
            const u16 ssg3 = (u16)op->ssg_hi * 100u + (u16)op->ssg_lo;
            sprintf(buf, "%03u", (unsigned)ssg3);
            VDP_drawText(buf, x + fm_op_col_x[9], y);
        }
        y++;
    }
}
