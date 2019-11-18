/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "Mmu.h"
#include "MmuZx81.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaZx81.h"



void MmuZx81::powerOn( /*t=0*/ int32 cc )
{
	MmuZx80::powerOn(cc);
	assert(romdis_in==0);

	uint n = rom.count(); if(n>8 kB) n=8 kB;
	cpu->mapRom(0,n,&rom[0],NULL,0);
}


/*	ROMCS Eingang wurde aktiviert/deaktiviert
	MMU.ROMCS handles paging of internal ROM.
	=> no need to forward it's own ROMCS state as well.

	In general items should not actively unmap their memory as result of incoming ROMCS.
	Instead they should trust that the sender will map it's rom into the Z80's memory space.
	While the ROMCS input is active, items must not page in their memory, just store values written to their registers only.

	ROM_CS:  1->disable
*/
void MmuZx81::romCS( bool f )
{
	xlogline("MmuZx81.romCS(%i)",f);

	if(f==romdis_in) return;
	romdis_in = f;
	if(f) return;		// paged out

	uint n = rom.count(); if(n>8 kB) n=8 kB;
	cpu->mapRom(0,n,&rom[0],NULL,0);
}





















