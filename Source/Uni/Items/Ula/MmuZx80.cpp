// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuZx80.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaZx80.h"



// dummy read page for unmapped cpu address space:
// no memory, flagged for cpu_crtc_zx81 => ula vram read
//
static CoreByte zx80_crtc_nomem[CPU_PAGESIZE];

ON_INIT([]
{
	xlogline("init zx80_crtc_nomem");
	CoreByte *p = zx80_crtc_nomem, *e = p+CPU_PAGESIZE;
	while(p<e) { *p++ = cpu_crtc_zx81 | 0x00ff; }	// flag + idle bus value
});


// -------------------------------------------------------------------------


void MmuZx80::powerOn(int32 cc)
{
	xlogIn("MmuZx80:init");
	Mmu::powerOn(cc);
	mapMem();
}

void MmuZx80::mapMem()
{
	//	ram = machine->ram;	// => shared array
	//	rom = machine->rom;	// => shared array

	cpu->unmapAllMemory();

	// 0x0000 - 0x3FFF: ROM
	switch (rom.count() / (2 kB))
	{
	case 2:	// 4 kB: ZX80
		// A12 must be 0 to select the internal ROM
		cpu->mapRom(0 kB/*addr*/, 4 kB/*size*/, &rom[0], nullptr, 0);
		cpu->mapRom(8 kB,         4 kB,         &rom[0], nullptr, 0); // mirror? TODO
		break;

	case 5:	// 10 kB: TK85
		// TODO: in which pages do the additional 2k actually show up?
		// assumed: it is mirrored 4 times.
		assert (machine->model == tk85);
		cpu->mapRom(0 kB/*addr*/, 10 kB/*size*/, &rom[0],    nullptr, 0);
		cpu->mapRom(10 kB,        2 kB,          &rom[8 kB], nullptr, 0); // mirror? TODO
		cpu->mapRom(12 kB,        2 kB,          &rom[8 kB], nullptr, 0); // mirror? TODO
		cpu->mapRom(14 kB,        2 kB,          &rom[8 kB], nullptr, 0); // mirror? TODO
		break;

	case 4: // 8 kB: ZX81 et.al.
		cpu->mapRom(0 kB/*addr*/, 8 kB/*size*/, &rom[0], nullptr, 0);
		cpu->mapRom(8 kB/*addr*/, 8 kB/*size*/, &rom[0], nullptr, 0); // mirror? TODO
		break;

	default:
		IERR();
	}

	// 0x8000-0xFFFF: preset unattached memory: cpu_crtc_zx81 = cpu liest NOP statt opcode
	// TODO: mirror?
	for( uint32 i = 32 kB; i < 64 kB; i += CPU_PAGESIZE )
	{
		cpu->mapRom( uint16(i), CPU_PAGESIZE, zx80_crtc_nomem, nullptr, 0 );
	}

	// 0x4000 … 0x7FFF: 16 kB RAM
	//	possible values:
	//	1k, 2k, 3k, 4k, 8k, ≥16k
	//	ZX80 internal 1kB: activated if A14==1  (Memory Pack can override CS signal)
	//	ZX81 internal 1kB: ?
	//	ZX81 U.S. internal 2kB: ?
	//	Sinclair 1-3k Ram Pack probably results in mirrors every 4k	TODO
	uint sz = ram.count();
	cpu->mapRam( 0x4000, uint16(min(sz,0xC000u)), ram.getData(), nullptr, 0 );
	if (ram.count() < 0x1000 && ram.count() != 0x400) sz = 0x1000;    // 4 kB

	// mirror memory:
	for (uint i = 0x4000+sz; i < 0x8000; i+= CPU_PAGESIZE)
	{
		cpu->mapMem(uint16(i), CPU_PAGESIZE, cpu->rdPtr(uint16(i-sz)), cpu->wrPtr(uint16(i-sz)), nullptr, 0);
	}
	if (sz <= 0x8000) for (uint32 i=0x4000; i<0x8000; i+= CPU_PAGESIZE)
	{
		cpu->mapMem(uint16(i+0x8000), CPU_PAGESIZE, cpu->rdPtr(uint16(i)), cpu->wrPtr(uint16(i)), nullptr, 0 );
	}

	// set cpu_crtc_zx81 flag:
	// lower 32K: ula patch OFF:
	for( uint pg=0; pg<CPU_PAGES/2; pg++ )
	{
		CoreByte* p = cpu->rdPtr(uint16(pg * CPU_PAGESIZE));
		for( int i=0; i<CPU_PAGESIZE; i++ ) { p[i] &= ~cpu_crtc_zx81; }
	}

	// upper 32K: ula patch ON:
	// due to mirroring the flag may also reappear in the lower 32k.
	// note: that is why in Z80options.h the Macro GET_INSTR must still test bit 15 of the PC.
	for (uint pg = CPU_PAGES/2; pg < CPU_PAGES; pg++)
	{
		CoreByte* p = cpu->rdPtr(uint16(pg * CPU_PAGESIZE));
		for (uint i = 0; i < CPU_PAGESIZE; i++ ) { p[i] |= cpu_crtc_zx81; }
	}
}













