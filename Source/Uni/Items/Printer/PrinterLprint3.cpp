// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "PrinterLprint3.h"


//    WoS:
//    #define P_LPRINT_ON                     0xfb    /* Page in LPRINT ROM */
//    #define B_LPRINT_ON                     0x84    /* ---- ---- 1--- -0-- */
//    #define P_LPRINT_OFF                    0x7b    /* Page out LPRINT ROM */
//    #define B_LPRINT_OFF                    0x84    /* ---- ---- 0--- -0-- */

static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

PrinterLprint3::PrinterLprint3(Machine* m)
:    Printer(m,isa_PrinterLprint3,external,o_addr,i_addr)
{
}
