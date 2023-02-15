// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Grafpad.h"


//    WoS:
//    /* Definitions for Grafpad (British Micro) */
//    #define P_GRAFPAD_PEN                   0xff3f  /* Pen up/down */
//    #define P_GRAFPAD_X                     0xffbf  /* Pen position X coordinate   */
//    #define P_GRAFPAD_Y                     0xff7f  /* Pen position Y coordinate   */


static cstr o_addr = 0;
static cstr i_addr = 0;


GrafPad::GrafPad(Machine* m) : Item(m, isa_GrafPad, isa_Item, external, o_addr, i_addr) {}
