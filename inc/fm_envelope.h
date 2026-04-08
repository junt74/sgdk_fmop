#ifndef FM_ENVELOPE_H
#define FM_ENVELOPE_H

#include "fm_display.h"

/**
 * BG_B 用エンベロープ描画の初期化（タイルパターン VRAM 転送・パレット・プレーンクリア）。
 * `SPR_init` とアルゴスプライト登録の後に呼ぶ。
 * 各 OP は `TILE_USER_INDEX+1` から 100 タイル×4 OP を使用（計 400 タイル。`TILE_MAX` との兼合いに注意）。
 * 折れ線色は PAL1 の 2〜5（OP1〜4）、背景タイルは PAL1 の 10。
 */
void fm_envelope_init(void);

/**
 * 各 OP を 10×10 マスに描画：無音→(AR)TL ピーク→(DR)SL サステイン→(SR) 同高→(RR)無音の 5 頂点を直線接続。
 * （Bresenham・タイル転送。Bitmap モードは未使用）
 */
void fm_envelope_draw(const FmPatch *patch);

#endif
