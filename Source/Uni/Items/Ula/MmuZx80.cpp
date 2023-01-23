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
//	ram  = machine->ram;	// => shared array
//	rom  = machine->rom;	// => shared array

	cpu->unmapAllMemory();

// 0x0000 - 0x3FFF: ROM
	cpu->mapRom(0/*addr*/,4 kB/*size*/,&rom[0],NULL,0);
	switch(rom.count()/(2 kB))
	{
	case 2:	// 4kB
			cpu->mapRom(4 kB,4 kB,&rom[0],NULL,0); // mirror rom
			break;
	case 5:	// 10 kB
			if(machine->model==tk85)	// TODO: in which pages do the additional 2k actually show up?
			{
//				cpu->mapRom( 8 kB,2 kB,&rom[8 kB],NULL,0);
//				cpu->mapRom(10 kB,2 kB,&rom[8 kB],NULL,0);
//				cpu->mapRom(12 kB,2 kB,&rom[8 kB],NULL,0);
//				cpu->mapRom(14 kB,2 kB,&rom[8 kB],NULL,0);
				// goto 8 kB
			}
			else IERR();
	case 4: // 8kB
			cpu->mapRom(4 kB,4 kB,&rom[4 kB],NULL,0);
			break;
	default:
			IERR();
	}


// 0x8000-0xFFFF: preset unattached memory: cpu_crtc_zx81 = cpu liest NOP statt opcode
	for( int32 i=32 kB; i<64 kB; i+=CPU_PAGESIZE ) { cpu->mapRom( i, CPU_PAGESIZE, zx80_crtc_nomem, NULL, 0 ); }


// 0x4000 - 0x7FFF: RAM
// möglich sind:
// 1k, 2k, 3k, 4k, 8k, ≥16k
// ZX80 internal 1kB: wird aktiviert bei A14==1
// ZX81 internal 1kB: ?
// ZX81 U.S. internal 2kB: ?
// Bei Anschluss des Sinclair 1-3k Ram ergibt sich wohl ein 4k Raster
	uint sz = ram.count();
	cpu->mapRam( 0x4000, min(sz,0xC000u), ram.getData(), NULL, 0 );
	if(ram.count()<0x1000 && ram.count()!=0x400) sz = 0x1000;    // 4 kB

// Speicher spiegeln:
	for( uint32 i=0x4000+sz; i<0x8000; i+= CPU_PAGESIZE )
	{
		cpu->mapMem( i, CPU_PAGESIZE, cpu->rdPtr(i-sz), cpu->wrPtr(i-sz), NULL, 0 );
	}
	if(sz<=0x8000) for( uint32 i=0x4000; i<0x8000; i+= CPU_PAGESIZE )
	{
		cpu->mapMem( i+0x8000, CPU_PAGESIZE, cpu->rdPtr(i), cpu->wrPtr(i), NULL, 0 );
	}

// cpu_crtc_zx81 flag setzen:
// zuerst untere 32K: ula patch OFF:
	for( int pg=0; pg<CPU_PAGES/2; pg++ )
	{
		CoreByte* p = cpu->rdPtr( pg*CPU_PAGESIZE );
		for( int i=0; i<CPU_PAGESIZE; i++ ) { p[i] &= ~cpu_crtc_zx81; }
	}

// danach obere 32K: ula patch ON:
// wg. Adressspiegelung wird das FLag auch wieder in einigen unteren cpu pages gesetzt
// note: deshalb muss in Z80options.h im Macro GET_INSTR auch der PC geprüft werden.
	for( int pg=CPU_PAGES/2; pg<CPU_PAGES; pg++ )
	{
		CoreByte* p = cpu->rdPtr( pg*CPU_PAGESIZE );
		for( int i=0; i<CPU_PAGESIZE; i++ ) { p[i] |= cpu_crtc_zx81; }
	}
}













