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


#include "Grafpad.h"


//    WoS:
//    /* Definitions for Grafpad (British Micro) */
//    #define P_GRAFPAD_PEN                   0xff3f  /* Pen up/down */
//    #define P_GRAFPAD_X                     0xffbf  /* Pen position X coordinate   */
//    #define P_GRAFPAD_Y                     0xff7f  /* Pen position Y coordinate   */


static cstr o_addr=0;
static cstr i_addr=0;


GrafPad::GrafPad(Machine *m) :
	Item(m,isa_GrafPad,isa_Item,external,o_addr,i_addr)
{
}
