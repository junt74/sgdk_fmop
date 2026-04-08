#ifndef FM_CONFIRM_NOTE_H
#define FM_CONFIRM_NOTE_H

#include <genesis.h>

void fm_confirm_note_init(void);

/** 現在の確認音程（半音番号）。o0 C=0 … o8 B=107。 */
u8 fm_confirm_note_get(void);

/** `docs/display_layout.md` の `NOTE: D4+` 形式。`buf` は十分なサイズ（例 16）。 */
void fm_confirm_note_format(char *buf, u16 buflen);

/**
 * B 押下中の D-Pad エッジで音程を変更（左右 ±1 半音、上下 ±1 オクターブ）。
 * B が押されていなければ何もしない。
 * @return 表示の更新が必要になったとき真。
 */
bool fm_confirm_note_step(u16 joy, u16 joy_prev);

#endif
