// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FdcD80.h"


//    WoS:
//    #define P_D80_CMD                       0x81    /* Command (write) */
//    #define P_D80_STATE                     0x81    /* State (read) */
//    #define P_D80_TRACK                     0x83    /* Track (read/write) */
//    #define P_D80_SECTOR                    0x85    /* Sector (read/write) */
//    #define P_D80_DATA                      0x87    /* Data (read/write) */
//    #define P_D80_SYSTEM                    0x89    /* System register (write)*/



static cstr o_addr = nullptr;//TODO
static cstr i_addr = nullptr;//TODO

FdcD80::FdcD80(Machine*m)
:    Fdc(m,isa_FdcD80,external,o_addr,i_addr)
{
}
