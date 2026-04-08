#ifndef FM_ALG_ICON_H
#define FM_ALG_ICON_H

#include <genesis.h>

void fm_alg_icon_init(void);

/** `alg` は 0〜7（YM2612 アルゴリズム）。 */
void fm_alg_icon_set(u8 alg);

#endif
