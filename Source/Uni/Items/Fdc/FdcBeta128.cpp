// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FdcBeta128.h"


/*	up to 4 Disk drives (KDOS)
*/


//  WoS:            http://www.worldofspectrum.org/faq/reference/ports.htm
//    #define P_TRDOS_CMD                     0x1f    /* Command */
//    #define P_TRDOS_STATE                   0x1f    /* State */
//    #define P_TRDOS_TRACK                   0x3f    /* Track */
//    #define P_TRDOS_SECTOR                  0x5f    /* Sector */
//    #define P_TRDOS_DATA                    0x7f    /* Data */
//    #define P_TRDOS_SYSTEM                  0xff    /* System */


static cstr o_addr = nullptr;//TODO
static cstr i_addr = nullptr;//TODO


FdcBeta128::FdcBeta128(Machine*m)
:    Fdc(m,isa_FdcBeta128,external,o_addr,i_addr)
{
}




