#ifndef FM_DISPLAY_H
#define FM_DISPLAY_H

#include <genesis.h>

/** 画面全体の表示原点（タイル座標）。ビルド後に実機で調整する。 */
#define FM_DISPLAY_OFFSET_X 2u
#define FM_DISPLAY_OFFSET_Y 1u

/**
 * 数値列幅（右詰め `%*u` 用）。最小値・最大値・十進桁数の根拠は docs/display_layout.md。
 */
#define FM_VAL_WIDTH_ALG 3u
#define FM_VAL_WIDTH_FB 2u
#define FM_VAL_WIDTH_AR 2u
#define FM_VAL_WIDTH_DR 2u
#define FM_VAL_WIDTH_SR 2u
#define FM_VAL_WIDTH_RR 2u
#define FM_VAL_WIDTH_SL 2u
#define FM_VAL_WIDTH_TL 3u
#define FM_VAL_WIDTH_K 1u
#define FM_VAL_WIDTH_ML 2u
#define FM_VAL_WIDTH_DT 1u
#define FM_VAL_WIDTH_SSG 3u

/** パラメータ名テキスト用パレット（PAL0 は数値の既定表示用）。 */
#define FM_LABEL_PALETTE PAL1
/** `FM_LABEL_PALETTE` のカラー 15（前景）。CRAM インデックス = パレット番号×16+15。 */
#define FM_LABEL_PALETTE_TEXT_CRAM_INDEX ((u16)(FM_LABEL_PALETTE * 16u + 15u))

/** FM 音色パッチ（OP 列は MDSDRV-mml コメント行の並びと同一）。 */
typedef struct
{
    union {
        struct
        {
            u8 ar, dr, sr, rr, sl, tl, k, ml, dt, ssg;
        };
        u8 raw[10];
    };
} FmOpParams;

typedef struct
{
    u8 alg;
    u8 fb;
    FmOpParams op[4];
} FmPatch;

/** `palette_grey` を `FM_LABEL_PALETTE` に複製し、カラー 15 を #444（RGB #444444）に設定。 */
void fm_display_palette_init(void);

void fm_display_draw(const FmPatch *patch);

#endif
