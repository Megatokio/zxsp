// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Internal or external memory
*/

#include "Memory.h"
#include "cpp/cppthreads.h"
#include "Machine.h"


#define MAXSIZE	0x100000u	// 1 MB


/*	Speicher für Ram oder Rom in einer Maschine.
	Unterstützt "shared copies", z.B. für Ula und Mmu -> internes Ram

	***Note***
	changes to the memory configuration of a machine
	are only allowed (1) from main thread and (2) while machine is locked
*/



/*	Creator for initially present memory
	e.g. built-in ram and rom
*/
Memory::Memory(Machine* machine, cstr name, uint size) noexcept
:
	_cnt(0),
	data(min(size,MAXSIZE)),
	name(newcopy(name)),
	machine(machine)
{
	assert(isMainThread());
	assert(machine != nullptr);
	assert(machine->is_locked());
	assert(size <= MAXSIZE);

	machine->memoryAdded(this);
}


Memory::~Memory()
{
	assert(isMainThread());
	assert(machine->is_locked());
	assert(_cnt == 0);

	if(machine->cpu) machine->cpu->unmapMemory(data.getData(),data.count());
	machine->memoryRemoved(this);
	delete[] name;
}


/*	shrink memory
	shrinks only, does nothing if new count ≥ old count
	unmaps all memory from cpu because memory is reallocated in this.data[].
*/
void Memory::shrink(uint new_cnt) noexcept	// shrinks only
{
	assert(isMainThread());
	assert(machine->is_locked());

	if(new_cnt >= data.count()) return;

	if(machine->cpu) machine->cpu->unmapMemory(data.getData(),data.count());
	data.shrink(new_cnt);
	machine->memoryModified(this);
}


/*	grow memory
	grows only, does nothing if new count ≤ old count
	unmaps all memory from cpu because memory is reallocated in this.data[].
*/
void Memory::grow(uint new_cnt) noexcept
{
	assert(isMainThread());
	assert(machine->is_locked());
	assert(new_cnt <= MAXSIZE);

	if(new_cnt <= data.count()) return;

	if(machine->cpu) machine->cpu->unmapMemory(data.getData(),data.count());
	data.grow(new_cnt);
	machine->memoryModified(this);
}
























