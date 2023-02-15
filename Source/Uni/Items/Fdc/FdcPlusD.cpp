// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FdcPlusD.h"

/*	Plus D Interface
	up to 2 Disk drives
	1 Centronics port
*/

//    Wos:
//    #define P_PLUSD_CMD                     0xe3    /* Command */
//    #define P_PLUSD_STATE                   0xe3    /* State */
//    #define P_PLUSD_PAGE                    0xe7    /* Memory paging */
//    #define P_PLUSD_TRACK                   0xeb    /* Track */
//    #define P_PLUSD_SYSTEM                  0xef    /* System register */
//    #define P_PLUSD_SECTOR                  0xf3    /* Sector */
//    #define P_PLUSD_PRINTER                 0xf7    /* Printer data/ready */
//    #define P_PLUSD_DATA                    0xfb    /* Data */


static cstr o_addr = nullptr; // TODO
static cstr i_addr = nullptr; // TODO

FdcPlusD::FdcPlusD(Machine* m) : Fdc(m, isa_FdcPlusD, external, o_addr, i_addr) {}
