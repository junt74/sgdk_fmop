#include <genesis.h>
#include "fm_confirm_note.h"

/** o0 C = 0、o8 B = 8*12+11 = 107。 */
#define FM_CONFIRM_SEMI_MIN 0u
#define FM_CONFIRM_SEMI_MAX 107u

/** 既定: o4 C */
#define FM_CONFIRM_SEMI_DEFAULT (4u * 12u)

static u8 s_semi = FM_CONFIRM_SEMI_DEFAULT;

void fm_confirm_note_init(void)
{
    s_semi = FM_CONFIRM_SEMI_DEFAULT;
}

u8 fm_confirm_note_get(void)
{
    return s_semi;
}

bool fm_confirm_note_step(u16 joy, u16 joy_prev)
{
    if (!(joy & BUTTON_B))
        return FALSE;

    const u16 pressed = (u16)(joy & (u16)~joy_prev);
    int delta = 0;

    if ((pressed & BUTTON_LEFT) || (pressed & BUTTON_RIGHT))
    {
        if (pressed & BUTTON_RIGHT)
            delta = 1;
        else
            delta = -1;
    }
    else if ((pressed & BUTTON_UP) || (pressed & BUTTON_DOWN))
    {
        if (pressed & BUTTON_UP)
            delta = 12;
        else
            delta = -12;
    }
    else
        return FALSE;

    int n = (int)s_semi + delta;
    if (n < (int)FM_CONFIRM_SEMI_MIN)
        n = (int)FM_CONFIRM_SEMI_MIN;
    else if (n > (int)FM_CONFIRM_SEMI_MAX)
        n = (int)FM_CONFIRM_SEMI_MAX;

    if (n == (int)s_semi)
        return FALSE;

    s_semi = (u8)n;
    return TRUE;
}

void fm_confirm_note_format(char *buf, u16 buflen)
{
    static const char *const PC[] = {
        "C", "C+", "D", "D+", "E", "F", "F+", "G", "G+", "A", "A+", "B",
    };
    (void)buflen;
    u8 s = s_semi;
    if (s > FM_CONFIRM_SEMI_MAX)
        s = FM_CONFIRM_SEMI_MAX;
    const u8 oct = s / 12u;
    const u8 pc = s % 12u;
    sprintf(buf, "NOTE: %s%u", PC[pc], (unsigned)oct);
}
