// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Manage the paging control register of a ZX Spectrum+ 128K
*/

#include "unix/FD.h"
#include "Mmu128k.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"
#include "Ula/Ula128k.h"



// ZX Spectrum 128K / +2 Memory Control
#define	o_addr_128	"0---.----.----.--0-"	// üblicher Port: 0x7ffd

// zx128k and +2: ram pages 1,3,5 and 7 are contended



/*	creator for use by derived classes:
	• MmuPlus3
*/
Mmu128k::Mmu128k ( Machine* m, isa_id id, cstr o_addr, cstr i_addr )
:
	Mmu( m, id, o_addr, i_addr )
{
	xlogIn("new Mmu128k");
}


Mmu128k::Mmu128k ( Machine* m )
:
	Mmu( m, isa_Mmu128k, o_addr_128, NULL )
{
	xlogIn("new Mmu128k");
}


void Mmu128k::powerOn(int32 cc)
{
	xlogIn("Mmu128k:init");

	Mmu::powerOn(cc);
	assert(ram.count()==0x20000);
	assert(rom.count()==0x08000);
	assert(ula->isA(isa_Ula128k));
//	setPort7ffd(0x00);
	port_7ffd = 0;
	page_mem_128k();
}


void Mmu128k::reset ( Time t, int32 cc )
{
	xlogIn("Mmu128k:reset");

	Mmu::reset(t,cc);
	setPort7ffd(0x00);
}


void Mmu128k::saveToFile( FD& fd ) const noexcept(false) /*file_error,bad_alloc*/
{
	Mmu::saveToFile(fd);
	fd.write_uchar(port_7ffd);
}


void Mmu128k::loadFromFile( FD& fd ) noexcept(false) /*file_error,bad_alloc*/
{
	xlogIn("Mmu128k::loadFromFile");

	Mmu::loadFromFile(fd);
	setPort7ffd( fd.read_uchar() );
}


/*	page in selected rom
	assumed: not (yet) locked to 48k
*/
inline void Mmu128k::page_rom_128k()
{
	assert(!romdis_in);

	int i = port_7ffd & 0x10;
	cpu->mapRom(0x0000, 0x4000, &rom[i*0x400], NULL, 0);
}


/*	page in selected ram at $c000
	assumed: not (yet) locked to 48k
	ram banks 1,3,5,7 are contended
*/
inline void Mmu128k::page_ram_128k()
{
	int n = port_7ffd & 0x07;
	if(n&1) cpu->mapRam( 3*0x4000, 0x4000, &ram[n*0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize() );
	else    cpu->mapRam( 3*0x4000, 0x4000, &ram[n*0x4000], NULL, 0 );
}


/*	page in rom and ram pages
	assumed: not locked to 48k
	ram banks 1,3,5,7 are contended
*/
inline void Mmu128k::page_mem_128k()
{
	page_ram_128k();
	cpu->mapRam( 2*0x4000, 0x4000, &ram[2*0x4000], NULL, 0 );
	cpu->mapRam( 1*0x4000, 0x4000, &ram[5*0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize() );
	if(!romdis_in) page_rom_128k();
}



/*	set port $7ffd
	override 48k lock
	also calls Ula128k::setPort7ffd()
*/
void Mmu128k::setPort7ffd ( uint8 byte )
{
	port_7ffd = byte;
	page_mem_128k();
	Ula128kPtr(ula)->setPort7ffd(byte);
}


void Mmu128k::output( Time, int32 /*cc*/, uint16 /*addr*/, uint8 byte )
{
	xlogIn("Mmu128k:Output($%2x)",uint(byte));

	if(port7ffdIsLocked()) return;					// mmu port disabled.

	uint8 toggled = port_7ffd ^ byte;
	port_7ffd = byte;

//	if( toggled&0x08 )									// screen changed? => handled by Ula128k
	if( toggled&0x07 ) page_ram_128k();					// ram page at $c000 changed?
	if( toggled&0x10 && !romdis_in) page_rom_128k();	// rom page changed?
}


//  ROMCS Eingang wurde aktiviert/deaktiviert
//	MMU.ROMCS handles paging of internal ROM.
//	=> no need to forward it's own ROMCS state as well.
//
void Mmu128k::romCS( bool f )
{
	if(f==romdis_in) return;	// no change
	romdis_in = f;
	if(f) return;				// rom disabled, caller will map it's own rom, no need to unmap it here

	page_rom_128k();
}
















