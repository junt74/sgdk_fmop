#include <genesis.h>

int main()
{
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    VDP_drawText("SGDK FMOP", 15, 13);

    while (1)
    {
        SYS_doVBlankProcess();
    }

    return 0;
}
