#ifndef FM_PREVIEW_H
#define FM_PREVIEW_H

#include <genesis.h>

#include "fm_display.h"

/** 復号 YM2612・Z80 バスを前提に 1 チャンネルで試聴用 FM を用意する。 */
void fm_preview_init(void);

/**
 * 毎フレーム呼ぶ。A 押下中のみ指定ノートで発音し、離すとキーオフ。
 * @param joy, joy_prev パッド（エッジ検出用）
 */
void fm_preview_frame(const FmPatch *patch, u16 joy, u16 joy_prev);

#endif
