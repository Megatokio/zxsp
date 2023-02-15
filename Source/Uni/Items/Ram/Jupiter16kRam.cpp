// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Jupiter16kRam.h"
#include "Items/Ula/Mmu.h"
#include "Machine.h"


/*  Jupiter 16k Ram Memory Extension
	erweitert den 3k Jupiter ACE auf 19k
*/


//  c'tor
//  note: Jupiter Forth will not use full ram unless reset
//
Jupiter16kRam::Jupiter16kRam(Machine* m) : ExternalRam(m, isa_Jupiter16kRam)
{
	machine->ram.grow(19 * 1024);
	machine->mmu->mapMem(); // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  Ace will most likely crash if not reset
//
Jupiter16kRam::~Jupiter16kRam()
{
	machine->ram.shrink(3 * 1024);
	machine->mmu->mapMem(); // map new memory to cpu & to set videoram
}
