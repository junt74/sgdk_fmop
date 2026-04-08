#ifndef FM_PREVIEW_H
#define FM_PREVIEW_H

#include <genesis.h>

#include "fm_display.h"

/** 試聴状態の初期化（`MDS_init` 後に呼ぶ）。 */
void fm_preview_init(void);

/**
 * A 押下中: MDSDRV の FM コマンドで現在パッチ・NOTE 行の音程を ch2 に発音。
 * A 離し: キーオフ。シーケンサと競合しないよう試聴中は全 `MDS_request` を止める。
 */
void fm_preview_frame(const FmPatch *patch, u16 joy, u16 joy_prev);

#endif
