// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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


#define o_addr nullptr
#define i_addr "----.----.--0-.----"


InvesJoy::InvesJoy(Machine* m) : KempstonJoy(m, isa_InvesJoy, internal, i_addr) { xlogIn("new InvesJoy"); }


// InvesJoy::~InvesJoy()
//{
//	xlogIn("~InvesJoy");
// }


// void InvesJoy::Input ( Time/*t*/, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
//{
//	// kempston issue 4 data bits:  %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 0
//	mask = 0xff;
//	byte = machine==frontMachine ? joystick()->getState() : 0x00;
// }
