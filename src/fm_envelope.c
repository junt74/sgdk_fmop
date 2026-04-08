#include <genesis.h>

#include "fm_display.h"
#include "fm_envelope.h"

/** グラフ 1 OP 幅・高さ（タイル数）。 */
#define FM_ENV_DIM 10u
/** 1 OP のグラフ領域（ピクセル）。10 タイル × 8px。 */
#define FM_ENV_PX (FM_ENV_DIM * 8u)

/**
 * 数値表の直後のタイル行（`fm_display_draw` の y 進行と一致）。
 * NOTE, ALG 行, ALG/FB 値, パラメータヘッダ, OP1〜4 の 8 行ぶん下。
 */
#define FM_ENV_BASE_TILE_Y (FM_DISPLAY_OFFSET_Y + 8u)

/** エンベロープは PAL1。背景インデックス 10、折れ線は OP に応じて 2〜5。 */
#define FM_ENV_PALETTE FM_MAIN_PALETTE
#define FM_ENV_PAL_CI_BG 10u
#define FM_ENV_LINE_OP0 2u

/** 全画面クリア用の単色タイル（従来どおり pattern 0）。 */
#define FM_ENV_TILE_FILL 0u
/**
 * OP0〜3 用に連続するタイル定義（各 OP 100 タイル = 10×10 マス分）。
 * pattern 1〜400 をエンベロープ専用に確保（方法1 Bitmap は使わずタイル書き換え）。
 */
#define FM_ENV_TILE_OP0 (1u)

/** インデックス FM_ENV_PAL_CI_BG（10 = ニブル 0xA）のベタタイル。 */
static const u32 s_tile_fill[8] = {
    0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,
    0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA,
};

static u16 tile_attr_fill(void)
{
    return TILE_ATTR_FULL(FM_ENV_PALETTE, FALSE, FALSE, FALSE,
                          (u16)(TILE_USER_INDEX + FM_ENV_TILE_FILL));
}

static void env_palette_colors(void)
{
    PAL_setColor((u16)(FM_ENV_PALETTE * 16u + FM_ENV_PAL_CI_BG), RGB24_TO_VDPCOLOR(0x000000u));
}

/**
 * AR / DR / SR / RR に応じた時間方向の重み（レートが速いほど区間が短い）。
 * 横 0〜79px に比例配分する。
 */
static void env_time_weights(u8 ar, u8 dr, u8 sr, u8 rr, u16 *wa, u16 *wd, u16 *ws, u16 *wr)
{
    *wa = 1u + (u16)(31u - ar) * 8u / 31u;
    *wd = 1u + (u16)(31u - dr) * 8u / 31u;
    *ws = 2u + (u16)sr * 6u / 31u;
    *wr = 1u + (u16)(15u - rr) * 6u / 15u;
}

/** 行 0＝最大寄り、行 9＝無音（従来どおり TL/SL で高さを決める）。 */
static s16 row_center_py(u8 row)
{
    return (s16)((u16)row * 8u + 4u);
}

static void plot_px(u8 *pix, s16 x, s16 y, u8 pen)
{
    if (x < 0 || y < 0 || x >= (s16)FM_ENV_PX || y >= (s16)FM_ENV_PX)
        return;
    pix[(u16)y * FM_ENV_PX + (u16)x] = pen;
}

/** Bresenham 直線（タイル用 4bpp ピクセルバッファへ点を打つ）。 */
static void bres_line(u8 *pix, s16 x0, s16 y0, s16 x1, s16 y1, u8 pen)
{
    const s16 dx = (s16)abs((int)(x1 - x0));
    const s16 dy = (s16)abs((int)(y1 - y0));
    const s16 sx = (x0 < x1) ? (s16)1 : (s16)-1;
    const s16 sy = (y0 < y1) ? (s16)1 : (s16)-1;
    s16 err = (s16)((int)dx - (int)dy);

    for (;;)
    {
        plot_px(pix, x0, y0, pen);
        if (x0 == x1 && y0 == y1)
            break;
        const s16 e2 = (s16)(err * 2);
        if (e2 > (s16)-dy)
        {
            err = (s16)(err - dy);
            x0 = (s16)(x0 + sx);
        }
        if (e2 < dx)
        {
            err = (s16)(err + dx);
            y0 = (s16)(y0 + sy);
        }
    }
}

/**
 * 5 頂点を直線で結ぶ: 無音開始 →(AR) 最大(TL) →(DR) サステイン(SL) →(SR) 同じ高さ →(RR) 無音。
 */
static void envelope_stroke_five(u8 *pix, const FmOpParams *op, u8 op_idx)
{
    const u8 bg = FM_ENV_PAL_CI_BG;
    const u8 fg = (u8)(FM_ENV_LINE_OP0 + (op_idx & 3u));
    const u16 n = FM_ENV_PX * FM_ENV_PX;
    for (u16 i = 0u; i < n; i++)
        pix[i] = bg;

    u16 wa, wd, ws, wr;
    env_time_weights(op->ar, op->dr, op->sr, op->rr, &wa, &wd, &ws, &wr);
    u16 sum = (u16)(wa + wd + ws + wr);
    if (sum == 0u)
        sum = 1u;

    const s16 x_max = (s16)(FM_ENV_PX - 1u);
    s16 x2 = (s16)((u32)79u * (u32)wa / (u32)sum);
    s16 x3 = (s16)((u32)79u * (u32)(wa + wd) / (u32)sum);
    s16 x4 = (s16)((u32)79u * (u32)(wa + wd + ws) / (u32)sum);
    if (x2 < 1) x2 = 1;
    if (x3 <= x2) x3 = (s16)(x2 + 1);
    if (x4 <= x3) x4 = (s16)(x3 + 1);
    if (x4 >= x_max) x4 = (s16)(x_max - 1);
    if (x3 >= x4) x3 = (s16)(x4 - 1);
    if (x2 >= x3) x2 = (s16)(x3 - 1);
    if (x2 < 1) x2 = 1;

    u8 peak_r = (u8)(((u16)op->tl * 9u) / 127u);
    u8 sus_r  = peak_r + (u8)(1u + ((u16)op->sl * (u8)(9u - peak_r)) / 15u);
    if (sus_r > 9u)
        sus_r = 9u;

    const s16 py0      = row_center_py(9u);
    const s16 py_peak  = row_center_py(peak_r);
    const s16 py_sus   = row_center_py(sus_r);

    const s16 x1 = 0;
    const s16 x5 = x_max;

    bres_line(pix, x1, py0, x2, py_peak, fg);
    bres_line(pix, x2, py_peak, x3, py_sus, fg);
    bres_line(pix, x3, py_sus, x4, py_sus, fg);
    bres_line(pix, x4, py_sus, x5, py0, fg);
}

/** ピクセルバッファ → `VDP_loadTileData` 用（タイル 1 行 = 4 バイト、先頭ピクセルが MSB 側）。 */
static u32 pack_tile_row(const u8 *pix, u8 tx, u8 ty, u8 row)
{
    const u16 y = (u16)ty * 8u + (u16)row;
    const u16 x0 = (u16)tx * 8u;
    u8 b[4];
    for (u8 i = 0u; i < 4u; i++)
    {
        const u16 x = x0 + (u16)i * 2u;
        const u8 p0 = pix[y * FM_ENV_PX + x];
        const u8 p1 = pix[y * FM_ENV_PX + (x + 1u)];
        b[i] = (u8)((p0 << 4) | (p1 & 0x0Fu));
    }
    return ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | (u32)b[3];
}

static void pix_to_tiles(const u8 *pix, u32 *tiles /* [100][8] flattened */)
{
    u16 idx = 0u;
    for (u8 ty = 0u; ty < FM_ENV_DIM; ty++)
    {
        for (u8 tx = 0u; tx < FM_ENV_DIM; tx++)
        {
            for (u8 r = 0u; r < 8u; r++)
                tiles[idx++] = pack_tile_row(pix, tx, ty, r);
        }
    }
}

static u16 tile_attr_op_cell(u8 op, u8 tx, u8 ty)
{
    const u16 tid = (u16)(TILE_USER_INDEX + FM_ENV_TILE_OP0 + (u16)op * (FM_ENV_DIM * FM_ENV_DIM) +
                          (u16)ty * FM_ENV_DIM + (u16)tx);
    return TILE_ATTR_FULL(FM_ENV_PALETTE, FALSE, FALSE, FALSE, tid);
}

/** 80×80 + 100×8×4；スタックを圧迫しないよう static。 */
static u8 s_pix[FM_ENV_PX * FM_ENV_PX];
static u32 s_tiles[FM_ENV_DIM * FM_ENV_DIM * 8u];

void fm_envelope_init(void)
{
    VDP_loadTileData(s_tile_fill, (u16)(TILE_USER_INDEX + FM_ENV_TILE_FILL), 1u, DMA);
    env_palette_colors();
    VDP_fillTileMapRect(BG_B, tile_attr_fill(), 0u, 0u, 40u, 28u);
}

void fm_envelope_draw(const FmPatch *patch)
{
    const u16 base_y = FM_ENV_BASE_TILE_Y;

    for (u16 op = 0u; op < 4u; op++)
    {
        envelope_stroke_five(s_pix, &patch->op[op], (u8)op);
        pix_to_tiles(s_pix, s_tiles);
        VDP_loadTileData(s_tiles, (u16)(TILE_USER_INDEX + FM_ENV_TILE_OP0 + op * FM_ENV_DIM * FM_ENV_DIM),
                         (u16)(FM_ENV_DIM * FM_ENV_DIM), DMA);

        const u16 ox = (u16)(op * FM_ENV_DIM);
        for (u16 ty = 0u; ty < FM_ENV_DIM; ty++)
        {
            for (u16 tx = 0u; tx < FM_ENV_DIM; tx++)
            {
                const u16 attr = tile_attr_op_cell((u8)op, (u8)tx, (u8)ty);
                VDP_setTileMapXY(BG_B, attr, (u16)(ox + tx), (u16)(base_y + ty));
            }
        }
    }
}
