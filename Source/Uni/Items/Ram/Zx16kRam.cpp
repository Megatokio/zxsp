// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Zx16kRam.h"
#include "Machine.h"
#include "Items/Ula/Mmu.h"


/*  Sinclair ZX 16K RAM Memory Extension
	erweitert den ZX81 auf 16k
	Das eingebaute RAM wird deaktiviert

	Note: TS 1500 wurde dadurch auf 32k erweitert!  TODO
*/


//protected
Zx16kRam::Zx16kRam(Machine* m,isa_id id)
:
   ExternalRam(m,id)
{
	xlogIn("new Zx16kRam");

	machine->ram.grow(16*1024);
	machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  c'tor
//  note: BASIC will not use full ram unless reset
//
Zx16kRam::Zx16kRam(Machine*m)
:
	ExternalRam(m,isa_Zx16kRam)
{
	xlogIn("new Zx16kRam");

	machine->ram.grow(16*1024);
	machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  ZX81 will most likely crash if not reset
//
Zx16kRam::~Zx16kRam()
{
	xlogIn("~Zx16kRam");

	machine->ram.shrink(machine->model_info->ram_size);
	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
}




