#include <genesis.h>

#include "fm_display.h"
#include "fm_cursor.h"
#include "fm_alg_icon.h"
#include "fm_confirm_note.h"
#include "fm_preview.h"
#include "mdsdrv.h"
#include "mdsdat.h"
#include "z80_ctrl.h"

static void mds_vint(void) { MDS_update(); }

int main()
{
    static FmPatch patch = {
        .alg = 3,
        .fb = 0,
        .op =
            {
                {.raw = {31, 0, 19, 5, 0, 23, 0, 0, 0, 0, 0}},
                {.raw = {31, 6, 0, 4, 3, 19, 0, 0, 0, 0, 0}},
                {.raw = {31, 15, 0, 5, 4, 38, 0, 4, 0, 0, 0}},
                {.raw = {31, 27, 0, 11, 1, 0, 0, 1, 0, 0, 0}},
            },
    };

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    JOY_init();
    SPR_init();

    Z80_unloadDriver();
    if (MDS_init(mdsseqdat, mdspcmdat) != 0)
        VDP_drawText("MDS_init failed", 2, 2);
    SYS_setVIntCallback(mds_vint);

    fm_display_palette_init();
    fm_preview_init();
    fm_confirm_note_init();
    fm_cursor_init();
    fm_alg_icon_init();
    fm_alg_icon_set(patch.alg);
    fm_display_draw(&patch);

    u16 joy_prev = 0u;

    while (1)
    {
        const u16 joy = JOY_readJoypad(JOY_1);
        bool redraw = FALSE;
        if (joy & BUTTON_B)
        {
            redraw = fm_confirm_note_step(joy, joy_prev);
            fm_cursor_refresh_sprite_on_note_row();
        }
        else
        {
            if (fm_cursor_step(joy, joy_prev, &patch))
                redraw = TRUE;
        }
        if (redraw)
            fm_display_draw(&patch);

        fm_preview_frame(&patch, joy, joy_prev);
        joy_prev = joy;

        fm_alg_icon_set(patch.alg);

        SPR_update();
        SYS_doVBlankProcess();
    }

    return 0;
}
