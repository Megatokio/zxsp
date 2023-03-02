// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuPlus3.h"
#include "Fdc/FdcPlus3.h"
#include "Machine.h"
#include "Printer/PrinterPlus3.h"
#include "Ula/UlaZxsp.h"
#include "UlaPlus3.h"
#include "Z80/Z80.h"
#include "unix/FD.h"


/*	WoS FAQ:
	Peripheral: ZX Spectrum +2A / +3 Primary Memory Control
	Port: 01-- ---- ---- --0-
	Peripheral: ZX Spectrum +2A / +3 Secondary Memory Control.
	Port: 0001 ---- ---- --0-

	$7ffd:	paging control register on 128 and +2, +3, +2A
	$1ffd:	on +3 and +2A the register $1ffd is also used.

The additional memory features of the 128K/+2 are controlled to by writes to port 0x7ffd.
the ZX128 will respond to any port address with bits 1 and 15 reset.
the zxplus3 will respond to those port addresses with bit 1 reset, bit 14 set and bit 15 reset .
Reading from 0x7ffd returns floating bus values.

+3/+2A: ram banks 4,5,6 and 7 are contended


port 0x7ffd bits:
zx128 or plus3 in normal mode:

	0-2: RAM page (0-7) to map into memory at 0xc000.

	3:	Select normal (0) or shadow (1) screen to be displayed. The normal screen is in bank 5, whilst the shadow screen
is in bank 7. Note that this does not affect the memory between 0x4000 and 0x7fff, which is always bank 5.

	4:	zx128: ROM select. ROM 0 is the 128k editor and menu system; ROM 1 contains 48K BASIC.
		zxplus3: Bit 4 is now the low bit of the ROM selection.

	5:	If set, lock mmu port. further writing to port 7ffd (and 1ffd) is inhibited.


memory map of zx128:

0xffff +--------+--------+--------+--------+--------+--------+--------+--------+
	   | Bank 0 | Bank 1 | Bank 2 | Bank 3 | Bank 4 | Bank 5 | Bank 6 | Bank 7 |
	   |        |        |(also at|        |        |(also at|        |        |
	   |        |        | 0x8000)|        |        | 0x4000)|        |        |
	   |        |        |        |        |        | screen |        | screen |
0xc000 +--------+--------+--------+--------+--------+--------+--------+--------+
	   | Bank 2 |        Any one of these pages may be switched in.
	   |        |
	   |        |
	   |        |
0x8000 +--------+
	   | Bank 5 |
	   |        |
	   |        |
	   | screen |
0x4000 +--------+--------+
	   | ROM 0  | ROM 1  | Either ROM may be switched in.
	   |        |        |
	   |        |        |
	   |        |        |
0x0000 +--------+--------+


Port 0x1ffd bits
zxplus3 only

	0:	Paging mode. 0=normal, 1=special (ram only)
	1:	In normal mode, ignored.
	2:	In normal mode, high bit of ROM selection. The four ROMs are:
		  ROM 0: 128k editor, menu system and self-test program
		  ROM 1: 128k syntax checker
		  ROM 2: +3DOS
		  ROM 3: 48 BASIC
	3:	Disk motor; 1=on, 0=off
	4:	Printer port strobe.

When special mode is selected, the memory map changes to one of four configurations specified in bits 1 and 2 of port
0x1ffd:

		 Bit 2 =0    Bit 2 =0    Bit 2 =1    Bit 2 =1
		 Bit 1 =0    Bit 1 =1    Bit 1 =0    Bit 1 =1
 0xffff +--------+  +--------+  +--------+  +--------+
		| Bank 3 |  | Bank 7 |  | Bank 3 |  | Bank 3 |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
		|        |  | screen |  |        |  |        |
 0xc000 +--------+  +--------+  +--------+  +--------+
		| Bank 2 |  | Bank 6 |  | Bank 6 |  | Bank 6 |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
 0x8000 +--------+  +--------+  +--------+  +--------+
		| Bank 1 |  | Bank 5 |  | Bank 5 |  | Bank 7 |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
		|        |  | screen |  | screen |  | screen |
 0x4000 +--------+  +--------+  +--------+  +--------+
		| Bank 0 |  | Bank 4 |  | Bank 4 |  | Bank 4 |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
		|        |  |        |  |        |  |        |
 0x0000 +--------+  +--------+  +--------+  +--------+
*/


#define o_addr_7ffd "01--.----.----.--0-" // üblicher Port: 0x7ffd
#define o_addr_1ffd "0001.----.----.--0-" // üblicher Port: 0x1ffd

static const int ram_only_mode_page_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 4, 5, 6, 3, 4, 7, 6, 3};


MmuPlus3::MmuPlus3(Machine* m) : Mmu128k(m, isa_MmuPlus3, And(o_addr_7ffd, o_addr_1ffd), nullptr)
{
	xlogIn("new MmuPlus3");
}


void MmuPlus3::reset(Time t, int32 cc)
{
	xlogIn("MmuPlus3:reset");

	Mmu::reset(t, cc); // bypass Mmu128k
	set_port_7ffd_and_1ffd(0, 0);
}


void MmuPlus3::powerOn(int32 cc)
{
	xlogIn("MmuPlus3:init");

	Mmu::powerOn(cc); // bypass Mmu128k
	assert(ram.count() == 0x20000);
	assert(rom.count() == 0x10000);
	//	set_port_7ffd_and_1ffd(0,0);
	port_7ffd = 0;
	port_1ffd = 0;
	page_mem_plus3(); // rom+ram mode
}


uint MmuPlus3::getPageC000() const volatile noexcept // for UlaInsp
{
	return isRamOnlyMode() ? ram_only_mode_page_table[2 * (port_1ffd & 6) + 3] : port_7ffd & 7;
}

uint MmuPlus3::getPage8000() const volatile noexcept // for UlaInsp
{
	return isRamOnlyMode() ? ram_only_mode_page_table[2 * (port_1ffd & 6) + 2] : 2;
}

uint MmuPlus3::getPage4000() const volatile noexcept // for UlaInsp
{
	return isRamOnlyMode() ? ram_only_mode_page_table[2 * (port_1ffd & 6) + 1] : 5;
}

uint MmuPlus3::getPage0000() const volatile noexcept // for UlaInsp
{
	return isRamOnlyMode() ? ram_only_mode_page_table[2 * (port_1ffd & 6) + 0] :
							 ((port_1ffd & 0x04) >> 1) + ((port_7ffd & 0x10) >> 4); // except if ROMDIS
}


//  ROMCS Eingang wurde aktiviert/deaktiviert
//	MMU.ROMCS handles paging of internal ROM.
//	=> no need to forward it's own ROMCS state as well.
//
void MmuPlus3::romCS(bool f)
{
	if (f == romdis_in) return; // no change
	romdis_in = f;
	if (f) return; // internal rom disabled: no need to unmap it here: caller will map it's rom anyway

	if (port_1ffd & 0x01) // ram only mode
	{
		// TODO: it's unclear whether ROMCS1 and ROMCS2 worked on the internal ram as well
		// probably not, but then we had bus collisions in the original machine anyway...

		// ram banks 4,5,6,7 are contended
		if (port_1ffd & 6)
			cpu->mapRam(0x0000, 0x4000, &ram[4 * 0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize());
		else cpu->mapRam(0x0000, 0x4000, &ram[0 * 0x4000], nullptr, 0);
	}
	else page_rom_plus3();
}


/*	page in selected rom
	assumes: not locked and not ram-only mode
*/
void MmuPlus3::page_rom_plus3()
{
	assert(!romdis_in);

	int c = ((int(port_1ffd & 0x04)) << 13)	   // hibit of rom_idx from bit 2 to bit 15
			+ ((int(port_7ffd & 0x10)) << 10); // lobit from bit 4 to bit 14

	cpu->unmapWom(0x0000, 0x4000);
	cpu->mapRom(0x0000, 0x4000, &rom[c], nullptr, 0);
}


/*	page in selected ram at $c000 in ROM+RAM mode
	assumes: not locked and not ram-only mode
*/
inline void MmuPlus3::page_ram_plus3()
{
	int n = port_7ffd & 0x07;
	// only ram banks 4,5,6,7 are contended				fixed kio 2016-02-11
	if (n >= 4) cpu->mapRam(3 * 0x4000, 0x4000, &ram[n * 0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize());
	else cpu->mapRam(3 * 0x4000, 0x4000, &ram[n * 0x4000], nullptr, 0);
}


/*	page in rom and ram pages in ROM+RAM mode
	assumes: not locked and not ram-only mode
*/
void MmuPlus3::page_mem_plus3()
{
	page_ram_plus3();
	// only ram banks 4,5,6,7 are contended				fixed kio 2016-02-11
	cpu->mapRam(2 * 0x4000, 0x4000, &ram[2 * 0x4000], nullptr, 0);
	cpu->mapRam(1 * 0x4000, 0x4000, &ram[5 * 0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize());
	if (!romdis_in) page_rom_plus3();
}


/*	page in 4 ram pages in ram-only mode
	assumes: ram-only mode
*/
void MmuPlus3::page_only_ram()
{
	const int* pp = ram_only_mode_page_table + 2 * (port_1ffd & 6);

	//	TODO: does ROMDIS also disable RAM at $0000 in ram only mode?
	//	Falls nicht, muss der ROMDIS-Setter auf only-ram mode prüfen!!
	//	Oder die MMU bietet SetRomDis(rom*) und ResRomDis() o.Ä. an.

	for (int i = 0; i < 4; i++)
	{
		// ram banks 4,5,6,7 are contended
		int n = pp[i];
		if (n >= 4)
			cpu->mapRam(i * 0x4000, 0x4000, &ram[n * 0x4000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize());
		else cpu->mapRam(i * 0x4000, 0x4000, &ram[n * 0x4000], nullptr, 0);
	}
}


/*	set port $7ffd and $1ffd
	override locked_to_48k
	• setup during initialisation
	• perform selections in control panels
	does not update screen up to current cc!
*/
void MmuPlus3::set_port_7ffd_and_1ffd(uint8 new_7ffd, uint8 new_1ffd)
{
	xlogIn("MmuPlus3:set_port_7ffd_and_1ffd:$%2x,$%2x", uint(new_7ffd), uint(new_1ffd));

	assert(dynamic_cast<Ula128k*>(ula));

	port_7ffd = new_7ffd;
	port_1ffd = new_1ffd;

	if (new_1ffd & 1) page_only_ram(); // ram only mode
	else page_mem_plus3();			   // rom+ram mode
	static_cast<Ula128k*>(ula)->setPort7ffd(new_7ffd);

	if (auto* p = dynamic_cast<PrinterPlus3*>(machine->printer)) p->strobe(new_1ffd & 0x10);
	if (machine->fdc) machine->fdc->setMotor(machine->now(), new_1ffd & 0x08);
}


void MmuPlus3::output(Time t, int32 /*cc*/, uint16 addr, uint8 byte)
{
	xlogIn("MmuPlus3:Output($%4x,$%2x)", uint(addr), uint(byte));

	if (port7ffdIsLocked()) return; // mmu ports locked

	if ((addr & 0xC002) == 0x4000) // write to port 7ffd
	{
		uchar toggled = port_7ffd ^ byte;
		port_7ffd	  = byte;

		//		if(toggled&0x08)					// screen changed? => handled in UlaPlus3
		if (port_1ffd & 1) return;							// +3/+2A: special mode: all ram, all set by port $1ffd only
		if (toggled & 0x07) page_ram_plus3();				// ram page at $c000 changed?
		if (toggled & 0x10 && !romdis_in) page_rom_plus3(); // rom page changed?
	}

	else if ((addr & 0xF002) == 0x1000) // write to port 1ffd
	{
		uchar toggled = port_1ffd ^ byte;
		port_1ffd	  = byte;

		if (byte & 1) // only ram
		{
			if (toggled & 7) page_only_ram(); // switch from rom+ram mode to ram-only mode
		}									  // or switched ram config. in ram-only mode
		else								  // rom+ram
		{
			if (toggled & 1) page_mem_plus3();					  // switch from ram only mode to rom+ram mode
			else if (toggled & 4 && !romdis_in) page_rom_plus3(); // switch rom in rom+ram mode
		}

		if ((toggled & 0x08) && machine->fdc) machine->fdc->setMotor(t, byte & 0x08);
		if (toggled & 0x10)
			if (auto* p = dynamic_cast<PrinterPlus3*>(machine->printer)) p->strobe(byte & 0x10);
	}
}
