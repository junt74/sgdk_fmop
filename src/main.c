#include <genesis.h>

#include "fm_display.h"
#include "fm_cursor.h"

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

    fm_display_palette_init();
    fm_cursor_init();
    fm_display_draw(&patch);

    u16 joy_prev = 0u;

    while (1)
    {
        const u16 joy = JOY_readJoypad(JOY_1);
        if (fm_cursor_step(joy, joy_prev, &patch))
            fm_display_draw(&patch);
        joy_prev = joy;

        SPR_update();
        SYS_doVBlankProcess();
    }

    return 0;
}
