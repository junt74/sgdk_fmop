#include <genesis.h>
#include <string.h>

#include "fm_display.h"

/** パラメータラベル前景色（#444 → #444444、SGDK 9bit 色へ変換）。 */
#define FM_LABEL_TEXT_RGB24 0x444444u

void fm_display_palette_init(void)
{
    PAL_setPalette(FM_LABEL_PALETTE, palette_grey, CPU);
    PAL_setColor(FM_LABEL_PALETTE_TEXT_CRAM_INDEX, RGB24_TO_VDPCOLOR(FM_LABEL_TEXT_RGB24));
}

/**
 * OP 数値列の先頭タイル X（`… K ML DT SSG` 相当の間隔で配置）。
 * パラメータ名行のみ `MLDT` と表示するため、ラベル「MLDT」と数値列の横位置は一致しない。
 */
static const u16 OP_COL_X[10] = {0u, 3u, 6u, 9u, 12u, 15u, 19u, 21u, 24u, 26u};

/** 各 OP パラメータの数値表示幅（AR … SSG の順、fm_display.h の FM_VAL_WIDTH_* と一致）。 */
static const u8 OP_VAL_WIDTH[10] = {
    FM_VAL_WIDTH_AR, FM_VAL_WIDTH_DR, FM_VAL_WIDTH_SR, FM_VAL_WIDTH_RR, FM_VAL_WIDTH_SL,
    FM_VAL_WIDTH_TL, FM_VAL_WIDTH_K, FM_VAL_WIDTH_ML, FM_VAL_WIDTH_DT, FM_VAL_WIDTH_SSG,
};

void fm_display_draw(const FmPatch *patch)
{
    char buf[16];
    u16 x = FM_DISPLAY_OFFSET_X;
    u16 y = FM_DISPLAY_OFFSET_Y;

    VDP_setTextPalette(FM_LABEL_PALETTE);
    sprintf(buf, "@0 fm %u %u", patch->alg, patch->fb);
    VDP_drawText(buf, x, y);
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
        for (u16 c = 0; c < 10; c++)
        {
            sprintf(buf, "%*u", (int)OP_VAL_WIDTH[c], (unsigned)op->raw[c]);
            VDP_drawText(buf, x + OP_COL_X[c], y);
        }
        y++;
    }
}
