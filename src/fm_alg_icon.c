#include <genesis.h>

#include "fm_alg_icon.h"
#include "fm_display.h"
#include "resources.h"

/** 1 フレーム幅（px）。6 タイル × 8px。 */
#define FM_ALG_ICON_W_PX 48
/** 画面右端からの余白（px）。 */
#define FM_ALG_ICON_PAD_R 8
/** 画面上端からの余白（px）。 */
#define FM_ALG_ICON_PAD_T 8
/** 右寄せ基準からさらに左へずらすタイル数（1タイル=8px）。 */
#define FM_ALG_ICON_SHIFT_LEFT_TILES 20u

static Sprite *s_alg;

void fm_alg_icon_init(void)
{
    const s16 x = (s16)(320 - FM_ALG_ICON_W_PX - FM_ALG_ICON_PAD_R -
                        (int)(FM_ALG_ICON_SHIFT_LEFT_TILES * 8u));
    const s16 y = (s16)FM_ALG_ICON_PAD_T;

    s_alg = SPR_addSprite(&spr_alg, x, y, TILE_ATTR(FM_MAIN_PALETTE, FALSE, FALSE, FALSE));
    SPR_setVisibility(s_alg, VISIBLE);
    SPR_setPriority(s_alg, FALSE);
}

void fm_alg_icon_set(u8 alg)
{
    u8 f = alg;
    if (f > 7u)
        f = 7u;
    SPR_setAnimAndFrame(s_alg, 0, (s16)f);
}
