// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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
	cpu->mapRom(0,n,&rom[0],nullptr,0);
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
	cpu->mapRom(0,n,&rom[0],nullptr,0);
}





















