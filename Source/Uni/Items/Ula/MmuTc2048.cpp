// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "unix/FD.h"
#include "MmuTc2048.h"
#include "Z80/Z80.h"
#include "Machine.h"
#include "Ula/UlaZxsp.h"


/*	Note: all ports fully decoded.

	Port F4 selects between EXROM and DOCK pages.
	These are not supported in the TC2048.
	Reading this port returns the last byte written to it.

	! Obwohl aus mehreren Quellen hervorzugehen scheint, dass Port F4 im TC2048 vorhanden ist
	! (und nur auf nicht existente Bänke umschaltet) scheint er nicht vorhanden zu sein:
	! "Basic64-Demo.tzx" funktioniert nur ohne. Es schaltet mutwillig die ROM-Bänke aus und stürzt dann ab.
	! Evtl. lässt sich das Register schreiben und wieder lesen - to be tested.

	The contended memory timings for these machines are unknown but should be similar to that for the 48K machine,
	except that the pattern starts at a different number of T-states after the interrupt, than the usual 14344.
*/


#define o_addr "----.----.1111.0100"
#define i_addr "----.----.1111.0100"


// Interrupt position:
// to be determined
//
#define cc_irpt_on	0
#define cc_irpt_off	64


// ----------------------------------------------------------
//					creator & init
// ----------------------------------------------------------

MmuTc2048::MmuTc2048(Machine*m, isa_id id, cstr oaddr, cstr iaddr)
:	MmuZxsp(m, id, oaddr, iaddr),
	port_F4(0)
{}

MmuTc2048::MmuTc2048(Machine*m)
:	MmuZxsp(m, isa_MmuTc2048, o_addr, i_addr),
	port_F4(0)
{}


void MmuTc2048::powerOn( int32 cc )
{
	MmuZxsp::powerOn(cc);
	assert(ula->isA(isa_UlaTc2048));
	port_F4 = 0;
}

void MmuTc2048::reset( Time t, int32 cc )
{
	MmuZxsp::reset(t,cc);
	port_F4 = 0;
}



// ----------------------------------------------------------
//					runtime
// ----------------------------------------------------------


/*	read port F4
	assumption: it exists but has no effect. see comment on Basic64-Demo.tzx
*/
void MmuTc2048::input( Time, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
{
	byte &= port_F4;
	mask  = 0xff;
}


/*	write port F4
*/
void MmuTc2048::output( Time, int32, uint16 /*addr*/, uint8 byte )
{
	port_F4 = byte;
}






