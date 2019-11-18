/*	Copyright  (c)	Günter Woigk 2006 - 2019
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

#include "InvesJoy.h"
#include "Machine.h"



/*	NOTE: currently this is a Kempston v.4
	TODO: actual Inves decoding
														kio 2007-04-29
	kempston:	issue 4, 1989

	address:	%xxxx.xxxx.000x.xxxx		&  /RD=0  &  /IORQ=0  &  /M1=1
				only A5, A6, A7 are decoded and compared to %000 via 74LS138

	data byte:	%000FUDLR active high		via 74LS366 hex inverting driver
				D5 = 0  via 6th driver
				D6 = 0  via diode
				D7 = 0  via diode

	acc. to http://www.zxprojects.com/index.php/the-fix-a-spectrum-blog/29-the-oddities-of-the-inves-spectrum
	the port is 223 ($DF)
*/


#define o_addr	NULL
#define	i_addr	"----.----.--0-.----"



InvesJoy::InvesJoy ( Machine* m )
:	KempstonJoy(m,isa_InvesJoy,internal,i_addr)
{
	xlogIn("new InvesJoy");
}


//InvesJoy::~InvesJoy()
//{
//	xlogIn("~InvesJoy");
//}


//void InvesJoy::Input ( Time/*t*/, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
//{
//	// kempston issue 4 data bits:  %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 0
//	mask = 0xff;
//	byte = machine==frontMachine ? joystick()->getState() : 0x00;
//}




























