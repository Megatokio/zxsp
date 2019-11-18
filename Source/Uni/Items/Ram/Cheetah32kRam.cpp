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



