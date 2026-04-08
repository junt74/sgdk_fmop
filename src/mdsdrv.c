//======================================================================
// MDSDRV API wrapper for SGDK
//======================================================================
// Copyright (c) 2020 Ian Karlsson
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must
// not claim that you wrote the original software. If you use this
// software in a product, an acknowledgment in the product
// documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must
// not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source
// distribution.
//======================================================================

#include <genesis.h>

#include "mdsdrv.h"

//! Sound driver work area.
u16 MDS_work[MDS_WORK_SIZE];

//! Initialize the sound driver
u16 MDS_init(const u8 *seqdata, const u8 *pcmdata)
{
    register u16 *a0 asm("a0") = MDS_work;
    register const u8 *a1 asm("a1") = seqdata;
    register const u8 *a2 asm("a2") = pcmdata;
    register u16 d0 asm("d0") = 0;
    asm volatile("jsr mdsdrvdat+0" : "+a"(a0), "+a"(a1), "=r"(d0) : "a"(a2) : "d1", "cc");
    return d0;
}

void MDS_request(u16 slot, u16 id)
{
    register u16 *a0 asm("a0") = MDS_work;
    register u16 d0 asm("d0") = id;
    register u16 d1 asm("d1") = slot;
    asm volatile("jsr mdsdrvdat+8" : : "r"(d0), "r"(d1), "a"(a0) : "cc");
}

u32 MDS_command(u16 id, u16 param)
{
    register u16 *a0 asm("a0") = MDS_work;
    register u32 d0 asm("d0") = id;
    register u16 d1 asm("d1") = param;
    asm volatile("jsr mdsdrvdat+12" : "+r"(d0), "+r"(d1) : "a"(a0) : "a1", "d2", "cc");
    return d0;
}

u32 MDS_command2(u16 id, u16 param1, u16 param2)
{
    register u16 *a0 asm("a0") = MDS_work;
    register u32 d0 asm("d0") = id;
    register u16 d1 asm("d1") = param1;
    register u16 d2 asm("d2") = param2;
    asm volatile("jsr mdsdrvdat+12" : "+r"(d0), "+r"(d1), "+r"(d2) : "a"(a0) : "a1", "cc");
    return d0;
}

void MDS_update(void)
{
    register u16 *a0 asm("a0") = MDS_work;
    register void *a6 asm("a6");
    asm volatile("jsr mdsdrvdat+4"
                 : "=a"(a6)
                 : "a"(a0)
                 : "a1", "a2", "a3", "a4", "a5", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc");
}

char *MDS_get_version_str(void)
{
    register u16 *a0 asm("a0") = MDS_work;
    register u32 d0 asm("d0") = MDS_CMD_GET_VERSION;
    asm volatile("jsr mdsdrvdat+12" : "+r"(d0), "+a"(a0) : : "a1", "d1", "d2", "cc");
    return (char *)a0;
}

void MDS_pause(u16 slot, bool state) { MDS_command2(MDS_CMD_SET_PAUSE, slot, state); }

void MDS_fade(u8 target, u8 speed, bool stop_when_done)
{
    MDS_command(MDS_CMD_FADE_BGM, (u16)((speed << 8) | ((stop_when_done & 1) << 7) | (target & 0x7f)));
}
