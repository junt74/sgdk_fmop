#ifndef FM_CURSOR_H
#define FM_CURSOR_H

#include <genesis.h>

#include "fm_display.h"

/** カーソル PNG（`pal_cursor_digit`）は nibbles 都合で専用ライン。本体内の他 UI は PAL1。 */
#define FM_CURSOR_PALETTE PAL3

void fm_cursor_init(void);

/**
 * `C` 非押下、または `C`＋十字の組み合わせで数値変更にならないとき: D-Pad でセル移動（エッジ検出）。
 * `C` 押下中かつ十字単独入力: カーソル位置の数値を変更（UP +10、DOWN -10、RIGHT +1、LEFT -1、クランプ）。
 * 押しっぱなしは初回のあと 20 フレーム待ち、以降 10 フレーム間隔でリピート。
 * @return パッチ数値を変更したとき真（画面の再描画が必要）。
 */
bool fm_cursor_step(u16 joy, u16 joy_prev, FmPatch *patch);

/** スプライト定義・座標を現在のセルに合わせる。 */
void fm_cursor_refresh_sprite(void);

/**
 * B 押下中に `main` から呼ぶ。ノート値（NOTE 行・プレフィックス直後）に 3 タイル幅カーソルを合わせる。
 */
void fm_cursor_refresh_sprite_on_note_row(void);

#endif
