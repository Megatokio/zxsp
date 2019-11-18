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




