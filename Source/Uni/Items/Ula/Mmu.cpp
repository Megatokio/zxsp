// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mmu.h"
#include "Machine.h"
#include "Z80/Z80.h"


/*	creator for use by derived classes:
 */
Mmu::Mmu(Machine* m, isa_id id, cstr o_addr, cstr i_addr) :
	Item(m, id, isa_Mmu, internal, o_addr, i_addr),
	cpu(m->cpu),
	ula(m->ula),
	ram(machine->ram), // => shared array
	rom(machine->rom)  // => shared array
{}


void Mmu::powerOn(int32 cc)
{
	Item::powerOn(cc);

	cpu = machine->cpu;
	assert(cpu);
	ula = machine->ula;
	assert(ula);
	//	ram  = machine->ram;	// => shared array
	//	rom  = machine->rom;	// => shared array
}
