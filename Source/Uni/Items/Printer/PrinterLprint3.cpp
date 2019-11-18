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
