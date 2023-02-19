// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuZx80.h"
#include "Machine.h"
#include "Ula/UlaZx80.h"
#include "Ula/UlaZx81.h"
#include "Z80/Z80.h"


void MmuZx80::powerOn(int32 cc)
{
	xlogIn("MmuZx80:powerOn");
	Mmu::powerOn(cc);
	mapMem();
}

void MmuZx80::mapMem()
{
	//	ram = machine->ram;	// => shared array
	//	rom = machine->rom;	// => shared array

	bool   is_zx81		= ula->isA(isa_UlaZx81);
	uint8* waitmap		= is_zx81 ? UlaZx81Ptr(ula)->getWaitmap() : nullptr;
	uint16 waitmap_size = is_zx81 ? UlaZx81Ptr(ula)->waitmap_size : 0;
	uint32 option		= is_zx81 ? cpu_waitmap : cpu_options_none; // cpu options without cpu_crtc_zx81

	// unmap all memory and set waitmap for ZX81:
	cpu->unmapRom(0, 0, waitmap, waitmap_size);
	cpu->unmapWom(0, 0, waitmap, waitmap_size);

	// flag dummy mem pages for cpu opcode patch:
	assert(NELEM(cpu->noreadpage) == CPU_PAGESIZE);
	assert(NELEM(cpu->nowritepage) == CPU_PAGESIZE);
	for (uint i = 0; i < CPU_PAGESIZE; i++)
	{
		cpu->noreadpage[i]	= option | cpu_crtc_zx81 | cpu_floating_bus | 0xFF;
		cpu->nowritepage[i] = option | cpu_crtc_zx81;
	}

	// range 0x0000 - 0x3FFF: map ROM
	switch (rom.count() / (2 kB))
	{
	case 2: // 4 kB: ZX80
		// A12 must be 0 to select the internal ROM
		cpu->mapRom(0 kB /*addr*/, 4 kB /*size*/, &rom[0], waitmap, waitmap_size);
		cpu->mapRom(8 kB, 4 kB, &rom[0], waitmap, waitmap_size); // mirror? TODO
		break;

	case 5: // 10 kB: TK85
		// TODO: in which pages do the additional 2k actually show up?
		// assumed: it is mirrored 4 times.
		assert(machine->model == tk85);
		cpu->mapRom(0 kB /*addr*/, 10 kB /*size*/, &rom[0], waitmap, waitmap_size);
		cpu->mapRom(10 kB, 2 kB, &rom[8 kB], waitmap, waitmap_size); // mirror? TODO
		cpu->mapRom(12 kB, 2 kB, &rom[8 kB], waitmap, waitmap_size); // mirror? TODO
		cpu->mapRom(14 kB, 2 kB, &rom[8 kB], waitmap, waitmap_size); // mirror? TODO
		break;

	case 4: // 8 kB: ZX81 et.al.
		cpu->mapRom(0 kB /*addr*/, 8 kB /*size*/, &rom[0], waitmap, waitmap_size);
		cpu->mapRom(8 kB /*addr*/, 8 kB /*size*/, &rom[0], waitmap, waitmap_size); // mirror? TODO
		break;

	default: IERR();
	}

	// range 0x4000 … 0x7FFF: map RAM
	//	possible values:
	//	1k, 2k, 3k, 4k, 8k, ≥16k
	//	ZX80 internal 1kB: activated if A14==1  (Memory Pack can override CS signal)
	//	ZX81 internal 1kB: ?
	//	ZX81 U.S. internal 2kB: ?
	uint sz = ram.count();
	cpu->mapRam(16 kB, uint16(min(sz, 48 kB)), ram.getData(), waitmap, waitmap_size);
	if (sz > machine->model_info->ram_size) sz = max(sz, 4 kB); // Sinclair 1-3k Ram Pack probably 4k mirrors

	// mirror memory:
	for (uint i = 16 kB + sz; i < 32 kB; i += CPU_PAGESIZE)
	{
		cpu->mapMem(
			uint16(i), CPU_PAGESIZE, cpu->rdPtr(uint16(i - sz)), cpu->wrPtr(uint16(i - sz)), waitmap, waitmap_size);
	}
	if (sz <= 16 kB)
		for (uint i = 0; i < 32 kB; i += CPU_PAGESIZE)
		{
			cpu->mapMem(
				uint16(32 kB + i), CPU_PAGESIZE, cpu->rdPtr(uint16(i)), cpu->wrPtr(uint16(i)), waitmap, waitmap_size);
		}

	// if zx81 then flag all memory for waitmap:
	if (option)
	{
		for (uint i = 0; i < rom->count(); i++) { rom[i] |= option; }
		for (uint i = 0; i < ram->count(); i++) { ram[i] |= option; }
	}

	// set cpu_crtc_zx81 flag in upper 32K:
	// due to mirroring the flag may also appear in the lower 32k.
	// that is why in Z80options.h the Macro GET_INSTR must still test bit 15 of the PC.
	for (uint page = CPU_PAGES / 2; page < CPU_PAGES; page++)
	{
		CoreByte* p = cpu->rdPtr(uint16(page * CPU_PAGESIZE));
		for (uint i = 0; i < CPU_PAGESIZE; i++) { p[i] |= option | cpu_crtc_zx81; }
	}
}
