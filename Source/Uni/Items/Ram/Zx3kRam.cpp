// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Zx3kRam.h"
#include "Items/Ula/Mmu.h"
#include "Machine.h"
#include "Memory.h"
#include "Qt/Settings.h"
#include <QSettings>
#include <QVariant>


// Sinclair ZX 1-3K RAM Memory Extension
// erweitert den ZX80 oder ZX81 um 1, 2 oder 3k
// der TS1000 wurde vermutlich unter Ignorierung seiner eingebauten 2k erweitert
// Effektiv wurde also der Speicher auf 2, 3 oder 4k erweitert.


Zx3kRam::Zx3kRam(Machine* m, uint sz) : ExternalRam(m, isa_Zx3kRam)
{
	xlogIn("new Zx3kRam");

	size = sz ? sz : settings.get_uint(key_zx3k_ramsize, 3 kB); // set in setRamSize()

	machine->ram.grow(1 kB + size);
	machine->mmu->mapMem(); // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  ZX80 will most likely crash if not reset
//
Zx3kRam::~Zx3kRam()
{
	xlogIn("~Zx3kRam");

	machine->ram.shrink(machine->model_info->ram_size);
	machine->mmu->mapMem(); // map new memory to cpu & to set videoram
}


// set ram size
// called from Zx3kInsp
// saves size in settings.
// restarts machine
//
void Zx3kRam::setRamSize(uint sz)
{
	assert(is_locked());

	xlogIn("Zx3kRam.setRamSize");

	if (sz == size) return;

	settings.setValue(key_zx3k_ramsize, sz);

	if (sz < size) machine->ram.shrink(1 kB + sz);
	if (sz > size) machine->ram.grow(1 kB + sz);
	size = sz;

	machine->mmu->mapMem(); // map new memory to cpu & to set videoram
							// machine->powerOn();		// initAllItems()
}
