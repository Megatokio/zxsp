/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/


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


static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO


FdcBeta128::FdcBeta128(Machine*m)
:    Fdc(m,isa_FdcBeta128,external,o_addr,i_addr)
{
}




