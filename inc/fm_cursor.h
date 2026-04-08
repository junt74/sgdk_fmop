#ifndef FM_CURSOR_H
#define FM_CURSOR_H

#include <genesis.h>

#include "fm_display.h"

/** カーソルスプライト用パレット（数値テキスト PAL0 とは別ライン。将来 PAL0 兼用も可）。 */
#define FM_CURSOR_PALETTE PAL2

void fm_cursor_init(void);

/**
 * `C` 非押下: D-Pad でセル移動（エッジ検出: `joy` と `joy_prev`）。
 * `C` 押下中: D-Pad でカーソル位置の数値を変更（UP +10、DOWN -10、RIGHT +1、LEFT -1、範囲内にクランプ）。
 * @return パッチ数値を変更したとき真（画面の再描画が必要）。
 */
bool fm_cursor_step(u16 joy, u16 joy_prev, FmPatch *patch);

/** スプライト定義・座標を現在のセルに合わせる。 */
void fm_cursor_refresh_sprite(void);

#endif
