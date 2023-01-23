// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "KempstonJoy.h"
#include "Machine.h"



/*														kio 2007-04-29
	kempston:	issue 4, 1989

	address:	%xxxx.xxxx.000x.xxxx		&  /RD=0  &  /IORQ=0  &  /M1=1
				only A5, A6, A7 are decoded and compared to %000 via 74LS138

	data byte:	%000FUDLR active high		via 74LS366 hex inverting driver
				D5 = 0  via 6th driver
				D6 = 0  via diode
				D7 = 0  via diode
*/


//    WoS:
//    #define P_KEMPSTON                      0x1f    /* Port address */
//    #define B_KEMPSTON                      0x20    /* ---- ---- --0- ---- */


//#define  o_addr	NULL
//#define  i_addr	"----.----.000-.----"       // Issue 4



KempstonJoy::KempstonJoy ( Machine* m, isa_id id, Internal i, cstr i_addr )
:	Joy(m,id,i,NULL,i_addr,"K")
{
	xlogIn("new KempstonJoy");
}


KempstonJoy::~KempstonJoy()
{
	xlogIn("~KempstonJoy");
}


void KempstonJoy::input ( Time/*t*/, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
{
	// kempston issue 4 data bits:  %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 0
	mask = 0xff;
	byte &= machine==front_machine ? joy[0]->getState() : 0x00;
}









