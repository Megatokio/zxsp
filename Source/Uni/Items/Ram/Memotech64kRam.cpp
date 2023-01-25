// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include <QSettings>
#include <QVariant>
#include "Memory.h"
#include "Qt/Settings.h"
#include "Memotech64kRam.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Items/Ula/Mmu.h"


/*  To set top of RAM at 64K type:

	POKE 16388,255  (this is not usually needed)
	POKE 16389 255  (this is usually set at 128 for a 32K limit)
	NEW             (the memory is now cleared to start afresh and will
					 now be organised to the new limit)

	To check the current top of RAM type:
	PRINT PEEK 16389

	MODE SWITCH
	---------------------------------------------------------------
		 1    2    3    4
	---------------------------------------------------------------
	A    ON   OFF  OFF  OFF  This mode provides 64K of memory and is for future developments
	B    OFF  ON   OFF  OFF  Memory is provided between 8K and 12K
	C    OFF  OFF  ON   OFF  Memory is provided between 12K and 16K
	D    OFF  OFF  OFF  ON   No memory is available between 8K and 16K
	E    OFF  ON   ON   OFF  Memory is available between 8K and 16K
	---------------------------------------------------------------

	For ZX81 users, switching between modes B, C, D, and E is possible, as long
	as at least one and no more than two switches are ON at the same time. Never
	have more than two switches on at a time, as this can lead to overloading.
*/


Memotech64kRam::Memotech64kRam(Machine*m)
:   ExternalRam(m,isa_Memotech64kRam)
{
	machine->ram.grow(64 kB);
	dip_switches = settings.get_int(key_memotech64k_dip_switches,0x06);

	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
	map_dip_switched_ram();
}


Memotech64kRam::~Memotech64kRam()
{
	machine->ram.shrink(machine->model_info->ram_size);

	prev()->romCS(false);
	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
}


/*  Ändere Dip-Switches
	Die Dip-Switches beeinflussen nur das Ram (oder Nicht-Ram) zw. 8k und 16k.
	Außer wenn Switch 1 ON ist, dann all-64k-ram.
	Genaues Error-Checking wird nicht gemacht.
	note: bits[3…0] == dip switch[1…4] lt. Memopak manual
*/
void Memotech64kRam::setDipSwitches(uint sw)
{
	assert(is_locked());

	if(sw==dip_switches) return;
	dip_switches = sw;

	settings.setValue(key_memotech64k_dip_switches,dip_switches);

	map_dip_switched_ram();
}


/*  Initialisiere dieses Item
	Die "normalen" 48k werden von MmuZx80/81 initialisiert
	Hier werden die unteren 16k nochmal umgebogen.
	Da MmuZx80/81 ab machine.ram[0] benutzt, benutzen wir hier das Ram ab machine.ram[48k]
	Note: Annahme: Ram, das nicht lesbar ist, ist auch nicht beschreibbar.
	Note: wenn im Bereich 8k .. 16k ROM ist, wird es hier durch RAM übersteuert.
		  In der realen Maschine hätten wir hier einen Bus-Konflikt.
*/
//virtual
void Memotech64kRam::powerOn(int32 cc)
{
	ExternalRam::powerOn(cc);
	map_dip_switched_ram();
}



/*	map ram in range 0-8k and 8-16k
*/
void Memotech64kRam::map_dip_switched_ram()
{
	if(dip_switches&8)			// all-64k-ram
	{
		prev()->romCS(true);
		machine->cpu->mapRam(0x0000,0x4000,&machine->ram[0xC000],nullptr,0);
	}
	else	// 0-8k: rom, 8-12 and 12-16k are ram or empty
	{
		prev()->romCS(false);
		if(dip_switches&4) machine->cpu->mapRam(0x2000,0x1000,&machine->ram[0xe000],nullptr,0);    // 8k-12k: Ram
		if(dip_switches&2) machine->cpu->mapRam(0x3000,0x1000,&machine->ram[0xf000],nullptr,0);    // 12k-16k: Ram
	}
}












