/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include "unix/FD.h"
#include "MmuTc2068.h"
#include "Z80/Z80.h"
#include "Machine.h"
#include "Ula/UlaZxsp.h"


/*
	#FE fully decoded

  The memory map of the 2068:

		 EX-ROM      HOME       DOCK
0xffff +----------+----------+----------+
	   |  Bank 7' | 32K RAM  |  Bank 7  |
	   |          |          |          |
0xe000 +----------+          +----------+
	   |  Bank 6' |          |  Bank 6  |
	   |          |          |          |
0xc000 +----------+          +----------+
	   |  Bank 5' |          |  Bank 5  |
	   |          |          |          |
0xa000 +----------+          +----------+
	   |  Bank 4' |          |  Bank 4  |
	   |          |          |          |
0x8000 +----------+----------+----------+
	   |  Bank 3' | Screen 1 |  Bank 3  |
	   |          |          |          |
0x6000 +----------+----------+----------+
	   |  Bank 2' | Screen 0 |  Bank 2  |
	   |          |          |          |
0x4000 +----------+----------+----------+
	   |  Bank 1' | 16K ROM  |  Bank 1  |
	   |          |          |          |
0x2000 +----------+          +----------+
	   |  Bank 0' |          |  Bank 0  |
	   |          |          |          |
0x0000 +----------+----------+----------+


port_ff:
	reading port_ff returns the last byte written to it.
	port_ff.bit0…5: set screen modes.										HANDLED IN ULA
	port_ff.bit6: If set disables the generation of the timer interrupt.	HANDLED IN ULA
	port_ff.bit7: determines which bank to use: 0=DOCK, 1=EX-ROM.			-> ULA calls selectExrom()


! Obwohl aus mehreren Quellen hervorzugehen scheint, dass Port F4 im TC2048 vorhanden ist (und nur auf nicht existente Bänke umschaltet)
! scheint er nicht vorhanden zu sein: "Basic64-Demo.tzx" funktioniert nur ohne. Es schaltet mutwillig die ROM-Bänke aus und stürzt dann ab.
! Evtl. lässt sich das Register schreiben und wieder lesen - to be tested.

Port_f4 determines which banks are to be paged in with each bit referring to the relevant bank EX-ROM or DOCK.
The HOME bank is the normal Spectrum memory area. The top 32K is uncontended but the 16K screen area below that is contended.
EX-ROM and DOCK banks are overlaid on the HOME bank, but paging over the screen area does not change the video ram used by the CRTC.
This does mean it is possible to set up a screen and page it out.

On a TC2048, BASIC is contained in the 16K ROM area and EX-ROM and DOCK banks are not normally available,
	while on a TS2068 or a TC2068 part of the BASIC is stored in an 8K ROM in DOCK bank 0
	and cartridges plugged into the dock use banks 0-7.

The contended memory timings for these machines are unknown but should be similar to that for the 48K machine,
except that the pattern starts at a different number of T-states after the interrupt, than the usual 14344.

Reading this port returns the last byte sent to it.
*/

/*******************************************************************
 *
 *      Bank switch between the 3 internal memory banks HOME, EXROM
 *      and DOCK (Cartridges). The HOME bank contains 16K ROM in the
 *      0-16K area and 48K RAM fills the rest. The EXROM contains 8K
 *      ROM and can appear in every 8K segment (ie 0-8K, 8-16K etc).
 *      The DOCK is empty and is meant to be occupied by cartridges
 *      you can plug into the cartridge dock of the 2068.
 *
 *      The address space is divided into 8 8K chunks. Bit 0 of port
 *      #f4 corresponds to the 0-8K chunk, bit 1 to the 8-16K chunk
 *      etc. If the bit is 0 then the chunk is controlled by the HOME
 *      bank. If the bit is 1 then the chunk is controlled by either
 *      the DOCK or EXROM depending on bit 7 of port #ff. Note this
 *      means that that the Z80 can't see chunks of the EXROM and DOCK
 *      at the same time.
 *
 *******************************************************************/

/*
	Bank #255 = HOME bank
	Bank #254 = EXROM bank
	Bank #0   = DOCK bank

	0xF4 = DKHSPT = Dock horizontal select
	0xFC = BDATPT = Expansion bank controller data
	0xFD = BCMDPT = Expansion bank controller address
	0xFF = HREXPT = Home Rom extension select bit (bit7)

	The HOME bank has the lowest priority and is paged in if no other bank overrides it.
	The DOCK and EXROM bank have the next highest priority and page in of not overridden by expansion banks.
	The expansion banks have the highest priority (probably by asserting ROMDIS)

bank controller registers:

	note: wird niemals angesprochen, außer durch AY port A bit 5 toggled => selectBusExpansionUnit()

	HOLD	temp. holding register
	BNA		bank number accessed: currently selected bank
	ABN		assigned bank number: bank number assigned to this bank		(one for each expansion bank)
	HS		horizontal select register: probably 1=enabled				(one for each bank)
	STATUS	status nibble:
			bit 0:  0 = bank caused an interrupt
			bit 1:		not used
			bit 2:  0 = bank can read/write data (memory present)
			bit 3:		not used

	port BCMDPT (0xFD bit 1,0) is used to select function of following port BDATPT (0xFC bit 3…0) access:
			read BDATPT 		write BDATPT
		0:	read status			write cmd type 1
		1:	--					write cmd type 2
		2:	read HS low nibble	write HOLD low nibble
		3:	read HS high nibble	write HOLD high nibble

	possible type 1 commands (bit 3…0):
		7:	reset interrupt flag
		11:	initialization (of bank) done. move to next bank in daisy chain
		13:	start interrupt REG sequence
		14:	reset controller - prepare to initialize

	possible type 2 commands (bit 3…0):
		12:	copy HOLD to HS
		13:	copy HOLD to BNA
		14:	copy HOLD to ABN
*/


// out:	1111.0100	F4
//		1111.1100	FC
//		1111.1101	FD
#define o_addr "----.----.1111.-10-"

// in:	1111.0100	F4
//		1111.1100	FC
#define i_addr "----.----.1111.-100"


// Interrupt position:
// to be determined
//
#define cc_irpt_on	0
#define cc_irpt_off	64



// ----------------------------------------------------------
//					creator & init
// ----------------------------------------------------------


MmuTc2068::MmuTc2068(Machine*m, isa_id id)
:
	MmuTc2048(m, id, o_addr, i_addr),
	cartridge(NULL),
	exrom_selected(no)
{
	const_cast<isa_id&>(grp_id) = isa_Fdc;		// => Inspector mit cmd-D und Toolwindow-Position wie +3 FDC
}


MmuTc2068::~MmuTc2068()
{
	delete cartridge;
}


void MmuTc2068::powerOn( int32 cc )
{
	MmuTc2048::powerOn(cc);

	assert(rom.count()==0x6000);		// HOME ROM + EXROM
	assert(ram.count()==0xC000);
	assert(ula&&cpu);
	assert(ula->isA(isa_UlaTc2048));

	exrom_selected = no;
	hold = 0;		// ?
	bna  = 0;		// ?
	port_FD = 0;	// ?
	set_port_f4(0,0xff);
}


void MmuTc2068::reset( Time t, int32 cc )
{
	MmuTc2048::reset(t,cc);

	exrom_selected = no;
	hold = 0;		// ?
	bna  = 0;		// ?
	port_FD = 0;	// ?
	set_port_f4(0,0xff);
}



// ----------------------------------------------------------
//					runtime
// ----------------------------------------------------------


void MmuTc2068::input( Time, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask )
{
	if(addr&0x08)	// port_FC = BDATPT
	{
		switch(port_FD&3)	// BCMDPT
		{
		case 0:		// read bank status
			break;	// bit0: no irpt pending, bit2: no memory present (note: there's even a pullup resistor at D2!)
		case 1:		// --
			break;	// --
		case 2:		// read HS low nibble
			break;	// there's no bank attached => floating bus?
		case 3:		// read HS high nibble
			break;	// there's no bank attached => floating bus?
		}
		// mask |= 0x0F;	no bank responding => no bits driven
		xlogline("TC2068: input(FC) = 0xFF");
	}
	else			// port_F4
	{
		xlogline("TC2068: input(F4) = 0x%02X",uint(port_F4));
		byte &= port_F4;
		mask  = 0xff;
	}
}


/*	Beschreibe Register port_F4
	schaltet zwischen HOME und EXROM/DOCK um
*/
void MmuTc2068::output( Time, int32, uint16 addr, uint8 byte )
{
// out:	1111.0100	F4
//		1111.1100	FC
//		1111.1101	FD
#define o_addr "----.----.1111.-10-"

	switch(addr&0x0F)
	{
	case 0x04:	// 0xF4
	  {
		uint8 toggled = byte ^ port_F4;
		if(toggled) set_port_f4(byte, toggled);
		break;
	  }
	case 0x05:	// --
		break;
	case 0x0d:	// 0xFD = BCMDPT = bank ctl. register address
		port_FD = byte;
		break;
	case 0x0c:	// 0xFC	= BDATPT = bank ctl. data
	  {
		switch(port_FD&3)
		{
		case 0:	// write cmd type 1
			switch(byte&0x0f)
			{
			case  7:
				xlogline("TC2068: reset interrupt flag");
				break;
			case 11:
				xlogline("TC2068: initialization (of bank) done. move to next bank in daisy chain");
				break;
			case 13:
				xlogline("TC2068: start interrupt REG sequence");
				break;
			case 14:
				xlogline("TC2068: reset controller - prepare to initialize");
				break;
			default:
				xlogline("TC2068: ill. type 1 command: 0x$02X", byte&0x0f);
				break;
			}
			break;
		case 1:	// write cmd type 2
			switch(byte&0x0f)
			{
			case 12:
				xlogline("TC2068: copy HOLD to HS (bank's page enable register): 0x$02X",uint(hold));
				// hs = hold;	no banks exist => no bank selected => no bank stores this value
				break;
			case 13:
				xlogline("TC2068: copy HOLD to BNA: 0x$02X",uint(hold));
				bna = hold;
				break;
			case 14:
				xlogline("TC2068: copy HOLD to ABN (bank's assigned bank number): 0x$02X",uint(hold));
				// abn = hold;	no banks exist => no bank selected => no bank stores this value
				break;
			default:
				xlogline("TC2068: ill. type 2 command: 0x$02X", byte&0x0f);
				break;
			}
			break;
		case 2:	// write HOLD low nibble
			xlogline("TC2068: write HOLD low nibble");
			hold &= 0xf0;
			hold |= byte&0x0f;
			xlogline("TC2068: write HOLD low nibble (HOLD=0x$02X)",uint(hold));
			break;
		case 3:	// write HOLD high nibble
			hold &= 0x0f;
			hold |= byte<<4;
			xlogline("TC2068: write HOLD high nibble (HOLD=0x$02X)",uint(hold));
			break;
		}
	  }
	}
}


/*	select EXROM or DOCK
	wird von UlaTc2048::setPortFF() aufgerufen
	Schaltet die Bänke, die laut port_F4 auf DOCK oder EXROM zeigen,
	zwischen DOCK und EXROM um.
	Bänke, die auf die HOME Bank zeigen, werden nicht beeinflusst.
*/
void MmuTc2068::selectEXROM(bool f)
{
	if(exrom_selected == f) return;				// keine Änderung

	xlogline("TC2068: select EXROM: %s",f?"on":"off");

	exrom_selected = f;
	if(port_F4) set_port_f4(port_F4, port_F4);
}


/*	callback from AY chip:
	port A bit 5 toggled
*/
void MmuTc2068::selectBusExpansionUnit(bool f)
{
	xlogline("MmuTc2068::selectBusExpansionUnit: %s",f?"disabled":"enabled"); (void)f;
}


/*	ROMCS Eingang wurde aktiviert/deaktiviert
	MMU.ROMCS handles paging of internal ROM.
	=> no need to forward it's own ROMCS state as well.

	In general items should not actively unmap their memory as result of incoming ROMCS.
	Instead they should trust that the sender will map it's rom into the Z80's memory space.
	While their backside ROMCS is active, items must not page in their memory, just store values written to their registers only.

	TODO: es ist unklar, auf welche Adressbereiche ROMCS am TC2048/TC2068 wirkt.
	Die aktuelle Annahme ist, dass ROMDIS nur auf das interne ROM wirkt.
	Wg. der Art der Implementierung der HOME Bank (sie wird in machine.rom geladen) wirkt es dadurch auch auf HOME Rom und Ram in einem TCC.

	ROM_CS:  1->disable
*/
void MmuTc2068::romCS( bool f )
{
	if(romdis_in==f) return;
	romdis_in = f;
	if(f) return;

	set_port_f4(port_F4,0x03);
}


/*	map memory acc. to register 0xF4

	assumption for romCS:
	romCS controls pages 0+1 for HOME, EXROM and DOCK
*/
void MmuTc2068::set_port_f4(uint8 f4_neu, uint8 toggled)
{
	xlogline("SetPortF4(0x%02X)", uint(f4_neu));

	port_F4 = f4_neu;

	if(f4_neu==0)			// nur HOME Banks:
	{
		if((toggled&3) && !romdis_in)
		{
			cpu->unmapWom(0x0000, 0x4000);
			cpu->mapRom(0x0000, 0x4000, &rom[0x0000], NULL, 0);
		}
		if(toggled&0x0C) cpu->mapRam(0x4000, 0x4000, &ram[0x0000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize());
		if(toggled&0xF0) cpu->mapRam(0x8000, 0x8000, &ram[0x4000], NULL, 0);
		return;
	}

	bool f = exrom_selected;
	MemoryPtr* xmem = 0;					// x = "extern" = EXROM oder DOCK
	uint8 xmem_r=0, xmem_w=0, home_w=0;		// home = HOME (im Modul!)

	if(cartridge)
	{
		xmem   = f ? &cartridge->exrom[0] : &cartridge->dock[0];
		xmem_r = f ?  cartridge->exrom_r  :  cartridge->dock_r;
		xmem_w = f ?  cartridge->exrom_w  :  cartridge->dock_w;
		home_w =	  cartridge->home_w;
	}

	for(uint i = romdis_in?2:0; i<8; i++)	// Schleife über 8 Bänke
	{
		uint mask = 1 << i;		if(~toggled & mask) continue;
		uint addr = i << 13;

		cuint8* waitmap = mask&0x0C ? ula_zxsp->getWaitmap() : NULL;
		uint    mapsize = mask&0x0C ? ula_zxsp->getWaitmapSize() : 0;

		if(f4_neu&mask)		// map in EXROM or DOCK
		{					// alle Bänke können Rom oder Ram enthalten:

			// reading:
			if(xmem_r&mask)	cpu->mapRom(addr,0x2000,xmem[i].getData(),waitmap,mapsize);	// Rom (or ram) in cartridge
			else if(f&&i==0)cpu->mapRom(addr,0x2000,&rom[0x4000],waitmap,mapsize);		// EXROM: map internal 8k EXROM		TODO: SMAS guide indicates bank #0 only
			else			goto h;														// map home bank
			//else			cpu->unmapRom(addr,0x2000);									// no memory here

			// writing:
			if(xmem_w&mask)	cpu->mapWom(addr,0x2000, xmem[i].getData(), waitmap,mapsize);	// Ram				TODO: evtl. write twice
			else if(i>=2)	cpu->mapWom(addr,0x2000, &ram[addr-0x4000], waitmap,mapsize);	//					TODO: evtl. don't write
			else			cpu->unmapWom(addr,0x2000);										// Rom
		}
		else				// HOME bank
		{					// in den unteren 16K können Ram und Rom liegen
							// in den restl. 48K darf nichts liegen: Nicht unterstützt.
							// Ich habe nur 2 Carts. mit Home-Bank gesehen, und die hatten jeweils nur 16K Rom, Rest leer oder Ram (fehlerhafterweise).

h:			if(i<2)			// internal rom address range
			{				// Im Cartridge könnte hier Ram sein:
				cpu->mapRom(addr,0x2000,&rom[addr],NULL,0);					// immer lesbar: entweder aus dem Cartridge oder aus dem internen Rom
				if(home_w&mask)	cpu->mapWom(addr,0x2000,&rom[addr],NULL,0);	// auch schreibbar? (also: Ram im Cartridge)
				else			cpu->unmapWom(addr,0x2000);					// nicht schreibbar
			}
			else			// internal ram address range
			{				// immer in/aus dem internen Ram!
				cpu->mapRam(addr,0x2000, &ram[addr-0x4000],waitmap,mapsize);
			}
		}
	}
}






void MmuTc2068::ejectCartridge()
{
	assert(is_locked());

	setPortF4(0);
	delete cartridge;
	cartridge = NULL;
}


void MmuTc2068::insertCartridge(cstr filepath)
{
	assert(is_locked());

	ejectCartridge();
	cartridge = new TccRom(machine,filepath);
}




