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


#include "AySubclasses.h"
#include "Machine.h"
#include "Joy/Joy.h"
#include "Ula/MmuTc2068.h"


// ------------------------------------------------------------------------
// ZX Spectrum 128k:
// ------------------------------------------------------------------------

#define zx128s "11--.----.----.--0-"		// ? select: üblicher Port: 0xfffd
#define zx128w "10--.----.----.--0-"		// ? write:  üblicher Port: 0xbffd
#define zx128r "11--.----.----.--0-"		// ? read:   üblicher Port: 0xfffd


AyForZx128::AyForZx128(Machine* m)
:	Ay(m, zx128s, zx128w, zx128r, m->model_info->ay_cycles_per_second, Ay::mono)
{}



// ------------------------------------------------------------------------
// TS2068, TC2068 and U2086:
// ------------------------------------------------------------------------

/*  The Timex TS2068 and TC2068 the joysticks are attached to port A of the AY-3-8912 sound chip.

  • When port A acts as an output port, the bits written to register 14 have the following meaning:

		bit 5:	0 = access to the (never released) 16MB Bus Expansion Unit is enabled.
		others:		might be used by 3rd party peripherals

  •	Reading port A while it is programmed as output:

		The byte read is masked with the port output byte: read = output & input
		e.g. the joystick bits D0-D4 return valid values only if they are not masked.

  • When port A is programmed as input port the value written to register 14 is ignored (not masked)
	and the bits read from register 14 have the following meaning:

		bit 0:		0 = up
		bit 1:		0 = down
		bit 2:		0 = left
		bit 3:		0 = right
		bit 4:		0 = fire
		bits 5-7:	111						TODO: SAMS says bit 7 is fire

	Address bits A8 and A9 determine which joystick will be read.
	If both bits are set then both joysticks are read with AND:		(TODO: WOS says: OR)

		bit 8:		1 = right joystick
		bit 9:		1 = left joystick
*/

#define tc2068s "----.----.1111.0101"		// ? select: üblicher Port: 0xF5
#define tc2068w "----.----.1111.0110"		// ? write:  üblicher Port: 0xF6
#define tc2068r "----.----.1111.0110"		// ? read:   üblicher Port: 0xF6


AyForTc2068::AyForTc2068(Machine*m)
:	Ay(m,tc2068s,tc2068w,tc2068r,m->model_info->ay_cycles_per_second,Ay::mono)
{}


// helper: convert %000FUDLR active high -> %111FRLDU active low
//
uint8 AyForTc2068::ayByteForJoystickByte(uint8 joy)
{
//	return ~( (joy&0x10)+((joy&8)>>3)+((joy&4)>>1)+((joy&2)<<1)+((joy&1)<<3) );
	return ~( ((joy&0x10)<<3)+((joy&8)>>3)+((joy&4)>>1)+((joy&2)<<1)+((joy&1)<<3) );
}

// callback from getRegister(): needs input value at port A pins:
//
uint8 AyForTc2068::getInputValueAtPortA(Time, uint16 addr)
{
	uint8 byte = 0xff;
	if(machine!=front_machine) return byte;
	if(addr&0x200) byte &= ayByteForJoystickByte(machine->joystick->joy[0]->getState()); // left
	if(addr&0x100) byte &= ayByteForJoystickByte(machine->joystick->joy[1]->getState()); // right
	return byte;
}

// notification from setRegister():
//
void AyForTc2068::portAOutputValueChanged(Time, uint8 newbyte)
{
	uint8 oldbyte = getOutputValueAtPortA();
	if((oldbyte^newbyte) & (1<<5))
	{
		bool f = (oldbyte>>5)&1;	// old=1 <=> new=0 <=> enabled=1
		MmuTc2068Ptr(machine->mmu)->selectBusExpansionUnit(f);
	}
}



// ------------------------------------------------------------------------
// Didaktik Melodik
// uses same port as ZX Spectrum 128K
// ------------------------------------------------------------------------

#define ms "11--.----.----.--0-"		// ? select: üblicher Port: 0xfffd
#define mw "10--.----.----.--0-"		// ? write:  üblicher Port: 0xbffd
#define mr "11--.----.----.--0-"		// ? read:   üblicher Port: 0xfffd


DidaktikMelodik::DidaktikMelodik(Machine* m)
:    Ay(m,isa_DidaktikMelodik,external,ms,mw,mr,1750000, Ay::acb_stereo)
{}



// ------------------------------------------------------------------------
// Zaxon AY-Magic
// uses same port as ZX Spectrum 128K
// ------------------------------------------------------------------------

#define zs "11--.----.----.--0-"		// ? select: üblicher Port: 0xfffd
#define zw "10--.----.----.--0-"		// ? write:  üblicher Port: 0xbffd
#define zr "11--.----.----.--0-"		// ? read:   üblicher Port: 0xfffd


ZaxonAyMagic::ZaxonAyMagic(Machine* m)
:    Ay(m,isa_ZaxonAyMagic,external,zs,zw,zr,1750000, Ay::acb_stereo/*TODO*/)
{}



// ------------------------------------------------------------
//			Bi-Pak ZON X Sound Box
// ------------------------------------------------------------

/*	AY-3-8192 sound chip

	xtal:			derived from /Clk pin at bus connector
		162500		ZX81

	connected lines:

		/clk
		/iorq
		/wr
		A0-4,A7			same for ZX81 and ZX Spectrum!

	Address decoding (ZON X issue 5):

		74LS11: triple 3-AND:

			and1 = A3+A2+A4
			and2 = A1+A0
			and3 = and1+nor1+and2

		74LS02: quad 2-NOR:

			nor1 = !( /wr | /iorq )
			nor2 = !A7
			nor3 = !( nor2 | nor4 )		= A7 + and1+nor1+and2
			nor4 = !and3				= !(and1+nor1+and2)

		AY-3-8912:
					   ______ ______
	ANALOG CHANNEL C =|1     U    28|= DA0
			  TEST 1 =|             |= DA1
			Vcc (+5v)=|             |= DA2
	ANALOG CHANNEL B =|             |= DA3
	ANALOG CHANNEL A =|             |= DA4
			Vss (Gnd)=|  AY-3-8912  |= DA5
				IOA7 =|             |= DA6
				IOA6 =|             |= DA7
				IOA5 =|             |= BC1			nor3 = A3+A2+A1+A0+A4+!(wr|iorq) + A7
				IOA4 =|             |= BC2			+5V
				IOA3 =|             |= BDIR			and3 = A3+A2+A1+A0+A4+!(wr|iorq)
				IOA2 =|             |= A8			+5V
				IOA1 =|             |= /RESET		power-on
				IOA0 =|14_________15|= CLOCK

	BC1     BDIR    Function

	0       0       Inactive
	0       1       Write to register		= A3+A2+A1+A0+A4+!(wr|iorq) + A7=0
	1       0       Read from register		= this is not possible
	1       1       Select register			= A3+A2+A1+A0+A4+!(wr|iorq) + A7=1



	used io addresses in ZX81 sample code:
		out	0xDF			select register
		out	0x0F			write to register (probably wrong, should be $1F)

	suggested io addresses for ZX Spectrum:
		out 0xFF			select register
		out 0x7F			write to register

	ZONX: TODO:	The sample code from the "ZON X" manual for the ZX81 should not work,
				because it sets A4=0 when writing to the register. ?!??
				The "ZON X-81" used port addresses $CF and $0F, where A4=0 in both cases.
				So this may be an error in the documentation,
				and the "ZON X" sample code for the ZX81 should use $DF and $1F.
*/

// Bi-Pak ZON X-81
#define s81 "----.----.1--0.1111"
#define w81 "----.----.0--0.1111"
#define r81 NULL

// Bi-Pak ZON X
#define s82 "----.----.1--1.1111"
#define w82 "----.----.0--1.1111"
#define r82 NULL

ZonxBox81::ZonxBox81( Machine*m )
: Ay( m,isa_ZonxBox81,external, s81, w81, r81, m->cpu_clock/2, Ay::mono )
{}

ZonxBox::ZonxBox( Machine*m )
: Ay( m,isa_ZonxBox,external, s82, w82, r82, m->cpu_clock/2, Ay::mono )
{}














