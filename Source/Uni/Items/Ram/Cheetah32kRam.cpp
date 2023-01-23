// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Cheetah32kRam.h"
#include "Machine.h"
#include "Items/Ula/Mmu.h"



/*  Cheetah 32k rampack
	erweitert den 16k Specci auf 48k
	darf nicht an einen 128k Specci angeschlossen werden
*/


//  c'tor
//  note: specci Basic will not use full ram unless reset
//
Cheetah32kRam::Cheetah32kRam(Machine*m)
:
	ExternalRam(m,isa_Cheetah32kRam)
{
	machine->ram.grow(48*1024);
	machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  specci will most likely crash if not reset
//
Cheetah32kRam::~Cheetah32kRam()
{
	machine->ram.shrink(machine->model_info->ram_size);
	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
}



