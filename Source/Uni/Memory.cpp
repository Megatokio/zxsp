/*	Copyright  (c)	Günter Woigk 2014 - 2018
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain_data the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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
	assert(machine != NULL);
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
























