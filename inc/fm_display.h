#ifndef FM_DISPLAY_H
#define FM_DISPLAY_H

#include <genesis.h>

/** 画面全体の表示原点（タイル座標）。ビルド後に実機で調整する。 */
#define FM_DISPLAY_OFFSET_X 2u
#define FM_DISPLAY_OFFSET_Y 4u
/** `fm_confirm_note_format` の「NOTE: 」の文字数（`"NOTE: %s%u"` のプレフィックス）。カーソル位置と共有。 */
#define FM_NOTE_PREFIX_LEN 6u
/** 確認ノートの値表示部（音名＋オクターブ）の最大タイル数。カーソル幅に使用。 */
#define FM_NOTE_VALUE_TILES 3u

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
/** SSG 数値表示は 3 桁（`%03u`、中身は `ssg_hi*100+ssg_lo`）。 */
#define FM_VAL_WIDTH_SSG 3u
/** カーソル用: SSG の 100 の位列（幅 1 タイル）。 */
#define FM_VAL_WIDTH_SSG_HI 1u
/** カーソル用: SSG の 10・1 の位列（幅 2 タイル、値 0〜15 → 表示 `00`〜`15`）。 */
#define FM_VAL_WIDTH_SSG_LO 2u

/** OP 論理列数（表示上の SSG は 3 桁 1 ブロックだが、カーソルは 100 の位と下 2 桁で分離）。 */
#define FM_OP_NUM_COLS 11u

/** OP 数値列の先頭タイル X（`fm_display_draw` / カーソル位置と共有）。 */
extern const u16 fm_op_col_x[FM_OP_NUM_COLS];
/** OP 各列の数値表示幅（文字数）。 */
extern const u8 fm_op_val_width[FM_OP_NUM_COLS];

/**
 * パラメータ名テキスト用パレット。
 * 数値テキストは `fm_display_draw` 内で PAL0（`VDP_setTextPalette(PAL0)`）を使用している。
 * カーソルスプライト（`res/cursor_*digit.png`）はファイル間で同一パレットであり、数値テキスト用の PAL0 と CRAM 上の 1 本のパレットラインとして併用できる。
 */
#define FM_LABEL_PALETTE PAL1
/** `FM_LABEL_PALETTE` のカラー 15（前景）。CRAM インデックス = パレット番号×16+15。 */
#define FM_LABEL_PALETTE_TEXT_CRAM_INDEX ((u16)(FM_LABEL_PALETTE * 16u + 15u))
/**
 * NOTE 行専用。ラベルは PAL1 の色 15 を #444 にしているが、描画ループ末尾で CRAM がその状態に
 * 戻るため、同じ色 15 を一時的に白へ切り替える方法ではフレーム全体が #444 になり見えない。
 * PAL3 の色 15 だけ白に固定し、`VDP_drawTextEx` でこのパレットを指定する。
 */
#define FM_NOTE_PALETTE PAL3

/** FM 音色パッチ（OP 列は MDSDRV-mml コメント行の並びと同一）。 */
typedef struct
{
    union {
        struct
        {
            u8 ar, dr, sr, rr, sl, tl, k, ml, dt;
            /** 十進 100 の位（0/1）。1 のとき MML では +100（AM）。 */
            u8 ssg_hi;
            /** SSG-EG 0〜15（下 2 桁、`%03u` で 00〜15）。 */
            u8 ssg_lo;
        };
        u8 raw[FM_OP_NUM_COLS];
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
