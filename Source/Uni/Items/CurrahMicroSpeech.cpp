// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include "CurrahMicroSpeech.h"
#include "globals.h"
#include "SP0256.h"
#include "Machine.h"
#include "Memory.h"
#include "Item.h"
#include "Items/Ula/Ula.h"


#define	uspeech_rom	"Roms/µspeech.rom"
#define sp0256_rom  "Roms/sp0256-al2.rom"	// the bytes in the file are in reversed bit-order,
											// ie. the "Target" values are unreversed,
											// all other opcodes and parameters are reversed

/*	uses memory mapped and no i/o ports, but /MREQ is not properly decoded:

	0x0038	address fully decoded
			but no control lines /M1, /RD and /MREQ						// verified by T. Busse
			=> activated by read, write, execute, input or output!
			TOGGLES (ENABLES/DISABLES) the ROM and also the i/o ports	// verified by T. Busse

	memory mapped i/o:
			DECODED: /RD, /WR and A12 to A15							// verified by T. Busse
			NOT DECODED: /MREQ
			this means, IN can be used instead of READ and OUT instead of WRITE,
			if you don't care about other peripherals. But as long as you use $FF for
			the low address byte this should work. Except on a TC2048…TS2068. B-)

	0x0000 - 0x07ff	the rom
	0x0800 - 0x0fff	a mirror of the rom									// verified by T. Busse

	0x1000	read:   bit  0 = /LRQ (load request) or RDY (ready)			// UNCLEAR: neither of both !!!?
					bits 1-7: floating. data read by the ZX Spectrum ULA?
			write:  bits 0-5: next allophone

	0x2000: nothing														// verified by T. Busse

	0x3000	write:  any value: set intonation (pitch) low				// ~3.05 MHz
	0x3001	write:  any value: set intonation (pitch) high				// ~7% higher

	; Disassembly of µSpeech rom:
	; register a contains the allophone and bit 6 for the intonation:
		ld de,$3000
		bit 6,a
		jr z,l1
		inc de
	l1:	ld (de),a		; set pitch
		ld ($1000),a	; write command
*/


#define	o_addr	"00-- ---- ---- ----"			// 0x38 = RST 7 fully decoded, $1xxx and $3xxx
#define	i_addr	"000- ---- ---- ----"			// 0x38 = RST 7 fully decoded, $1xxx

#define TOGGLE_ROM_ADDRESS		0x0038

// bytes (6 bits) written to this address are played:
// reading returns the busy state: bit0=1 => busy  (either LRQ or SBY)
#define WRITE_COMMAND_ADDRESS	0x1000
#define WRITE_COMMAND_MASK		0xF000			// 1101 0000 0000 0000
#define READ_STATE_ADDRESS		0x1000
#define READ_STATE_MASK			0xF000

// writing to $3000 or $3001 set's intonation (pitch) to low or high
#define SET_INTONATION_LOW_ADDRESS	0x3000
#define SET_INTONATION_HIGH_ADDRESS	0x3001
#define SET_INTONATION_ADDRESS		0x3000
#define SET_INTONATION_MASK			0xF000		// 1110 0000 0000 0001

// volumes and clock frequency:
static const Sample volume = 1.0;
static const Frequency clock_low = 3.05e6;
static const Frequency clock_high = clock_low * 1.07;

// output filter:
static const float RC = 11e3f * 22e-9f * 0.5f;			// 33kΩ * 22nF * 0.5


CurrahMicroSpeech::CurrahMicroSpeech(Machine* m)   // CREATOR
:
	Item(m,isa_CurrahMicroSpeech,isa_CurrahMicroSpeech/*grp*/,external,o_addr,i_addr),
	sp0256(NULL),
	rom(m,"Currah µSpeech Rom",2 kB),
	enable_state(no),
	pitch(0x00),
	clock(clock_low),
	current_clock(clock_low),
	lastrp(0),
	lastwp(0),
	pause(0)
{
	xlogIn("new CurrahMicroSpeech");

	// load rom:
	uint8 zbu[2048];
	FD fd(catstr(appl_rsrc_path,uspeech_rom));
	fd.read_bytes(zbu,2048);
	m->cpu->b2c(zbu,rom.getData(),2048);

	// create sound chip:
	sp0256 = new SP0256(catstr(appl_rsrc_path,sp0256_rom), no, RC);
}

CurrahMicroSpeech::~CurrahMicroSpeech()   // DESTRUCTOR
{
	xlogIn("~CurrahMicroSpeech");
	delete sp0256;
	machine->cpu_options &= ~cpu_memmapped_rw;		// assuming no other item with memory mapped io attached

	machine->ula->setBeeperVolume(1.0);
}

void CurrahMicroSpeech::powerOn(/*t=0*/ int32 cc) throws/*bad alloc*/
{
	//	power-on the device:
	//	set flags in memory so that our memory mapped i/o is called.

	xlogIn("CurrahMicroSpeech::init");

	Item::powerOn(cc);

	enable_state = off;
	machine->cpu_options |= cpu_patch | cpu_memmapped_rw;

	clock = current_clock = clock_low;
	sp0256->powerOn(volume, current_clock);
	machine->ula->setBeeperVolume(0.5f);			// mute the beeper while µspeech is attached

	// ideally we should set the cpu_patch bit in all roms in the system		TODO
	// and evtl. in rams too, e.g. for the +3. (but it won't work there probably anyway)
	// currently the µspeech will not work with external roms, e.g. IF2 or SPECTRA.
	// we'd need a way to iterate over all roms/memory, e.g. by defining a base class for items with rom.
	// else we could iterate over all items and patch at least those we know.
	// Many extensions with rom had no rear-side bus connector anyway.

	// register memory mapped i/o addresses
	// ASSUMPTION:
	// µspeech i/o is only enabled after RST 7	=> no need to add flags to machine rom
	// A14 and A15 are decoded					=> no need to add flags to ram
	// µspeech rom is only visible at 0x0XXX	=> no need to add flags to µspeech rom
	// A0 … A11 are NOT decoded					=> need to add flags to all unmapped memory

	// set mem_mapped_io flag cpu_patch for RST7 in µspeech rom:
	rom[0x0038] |= cpu_patch | cpu_memmapped_rw;

	// set mem_mapped_io flag and cpu_patch for RST7 in the machine's internal rom:
	MemoryPtr internalrom = machine->rom;
	for(uint i=0; i<internalrom.count(); i+=0x4000) { internalrom[i+0x38] |= cpu_patch | cpu_memmapped_rw; }

	// set mem_mapped_io flag in unmapped memory:
	CoreByte* noreadpage  = machine->cpu->noreadpage;
	CoreByte* nowritepage = machine->cpu->nowritepage;
	for(uint i=0;i<CPU_PAGESIZE; i++)
	{
		noreadpage[i] |= cpu_memmapped_r;
		nowritepage[i] |= cpu_memmapped_w;
	}

	this->prev()->romCS(off);
}

void CurrahMicroSpeech::reset(Time t, int32 cc)
{
	Item::reset(t,cc);
	clock = clock_low;
	sp0256->reset(t,volume,current_clock);
}

void CurrahMicroSpeech::audioBufferEnd( Time t )
{
	xxlogIn("CurrahMicroSpeech::audioBufferEnd");

	if(fabs(clock - current_clock) > 10)
	{
		current_clock += (clock-current_clock) * seconds_per_dsp_buffer() / 0.1;
		sp0256->setClock(t,current_clock);
	}

	bool stand_by = sp0256->audioBufferEnd(t);
	if(stand_by) pause += 2;		// Pause 2*64/10kHz
}

void CurrahMicroSpeech::toggle_enable_state()
{
	//	enable or disable rom and i/o ports
	//	any access to address 0x0038 toggles the enable state of the rom and i/o ports

	xxlogIn("CurrahMicroSpeech::toggle_enable_state() --> %s", enable_state?"enabled":"disabled");

	enable_state = !enable_state;

	prev()->romCS(enable_state);	// switch 0=on or 1=off internal rom
									// mapping internal rom will unmap our rom
									// unmapping internal rom will not automagically map our rom
	if(enable_state)
	{
		// the rom is visible at 0x0000 and mirrored at 0x0800

		machine->cpu->mapRom(0x0000,0x800,rom.getData(),NULL,0);
		machine->cpu->mapRom(0x0800,0x800,rom.getData(),NULL,0);	// mirror at 0x0800

		machine->cpu->unmapRom(0x1000,0x3000);						// unmap the remainder

		assert(machine->cpu_options & cpu_memmapped_r);
		assert(machine->cpu_options & cpu_memmapped_w);
		assert(machine->cpu->noreadpage[0] & cpu_memmapped_r);
		assert(machine->cpu->nowritepage[0] & cpu_memmapped_w);
		//assert(machine->cpu->getPage(0x0000).core_r == noreadpage-0x0000);
		//assert(machine->cpu->getPage(0x0000).core_w == nowritepage-0x0000);
		assert(machine->cpu->getPage(0x1000).core_r == machine->cpu->noreadpage-0x1000);
		assert(machine->cpu->getPage(0x1000).core_w == machine->cpu->nowritepage-0x1000);
		//assert(machine->cpu->getPage(0x2000).core_r == noreadpage-0x2000);
		//assert(machine->cpu->getPage(0x2000).core_w == nowritepage-0x2000);
		assert(machine->cpu->getPage(0x3000).core_r == machine->cpu->noreadpage-0x3000);
		assert(machine->cpu->getPage(0x3000).core_w == machine->cpu->nowritepage-0x3000);
	}
}

void CurrahMicroSpeech::input( Time t, int32, uint16 addr, uint8& byte, uint8& mask )
{
	// handle in(0x0038)
	// acc. to research of Thomas Busse the enable state toggles with in, out, read and write at address 0x0038
	// NOTE: nocash says that Rockfall (Crash) used this to detect the µSpeech
	// 	  but this port is never read by Rockfall. (kio 2017-10-04)
	// 	  it would be a very poor solution because this address also selects the ULA
	// 	  and maybe other periperals. (6 out of 8 low address bits a 0)
	// Thomas Busse says, Rockfall II used it:
	//	  E5AE  xor  a
	//    E5AF  in   a,($38)
	//    E5B1  ld   a,($0039)
	//    E5B4  cp   $F1         ; value in Currah rom: $F1; value in Sinclair rom: $E5

	if(addr == 0x0038)
	{
		logIn("CurrahMicroSpeech::input(0x0038)");
		toggle_enable_state();
		return;
	}

	if(!enable_state) return;

	// acc. to research of Thomas Busse WRITE and OUT are just the same:

	else if((addr & READ_STATE_MASK) == READ_STATE_ADDRESS)  // sp0256 state
	{
		bool busy = sp0256->isBusy(t);
		xlogIn("CurrahMicroSpeech::input(0x%04x) --> sp0256 is %s",addr,busy?"busy":"not busy");
		if(!busy) byte &= 0xFE;
		mask |= 0x01;
	}
}

void CurrahMicroSpeech::output( Time t, int32, uint16 addr, uint8 byte )
{
	if(addr == 0x0038)
	{
		// acc. to research of Thomas Busse the enable state
		// toggles with in, out, read and write at address 0x0038

		logIn("CurrahMicroSpeech::output(0x0038)");
		toggle_enable_state();
		return;
	}

	if(!enable_state) return;

	// acc. to research of Thomas Busse WRITE and OUT are just the same:

	else if((addr & WRITE_COMMAND_MASK) == WRITE_COMMAND_ADDRESS)  // sp0256 command
	{
		xlogIn("CurrahMicroSpeech::output: 0x%04x = 0x%02x --> command",addr,byte);

		byte &= 0x3F;			// bits[0…5] = command
		sp0256->writeCommand(t,byte);
		add_history(byte);
	}

	else if((addr & SET_INTONATION_MASK) == SET_INTONATION_ADDRESS)  // set pitch
	{
		if(addr & 1)
		{
			xlogIn("CurrahMicroSpeech:output(0x%04x) --> set intonation HIGH",addr);
			pitch = 0x40;
			clock = clock_high;
		}
		else
		{
			xlogIn("CurrahMicroSpeech:output(0x%04x) --> set intonation LOW",addr);
			pitch = 0x00;
			clock = clock_low;
		}
	}
}

uint8 CurrahMicroSpeech::handleRomPatch( uint16 pc, uint8 opcode )
{
	//	handle instruction at 0x0038 = RST7 = Interrupt
	//	return opcode from new rom
	//	this is the normal way used in the Currah rom

	if(pc == 0x0038)
	{
		xxlogIn("CurrahMicroSpeech::handleRomPatch(0x0038)");
		toggle_enable_state();
		return machine->cpu->peek(pc);	// re-read instruction from new rom
	}

	// execute READ_STATE address:
	// this will probably read the status bit and mix it into the floating bus byte
	// this requires to set the cpu_patch bit in all noreadpage[] cells
	// this just makes a program crash slightly different.
	//
	//if(enable_state && (pc & READ_STATE_MASK) == READ_STATE_ADDRESS)
	//	return sp0256->isBusy(t) ? opcode : opcode & 0xFE;

	return prev()->handleRomPatch(pc,opcode);		// not me
}

/*void CurrahMicroSpeech::romCS( bool f )
{
	// ROMCS Eingang wurde aktiviert/deaktiviert
	// MMU.ROMCS handles paging of internal ROM.
	// => no need to forward it's own ROMCS state as well.
	//
	// In general items should not actively unmap their memory as result of incoming ROMCS.
	// Instead they should trust that the sender will map it's rom into the Z80's memory space.
	// While their backside ROMCS is active, items must not page in their memory,
	// just store values written to their registers only.
	//
	// f=1 -> disable internal rom
	//
	// note: Currah µSpeech hatte keinen durchgeschleiften Bus

	if(f==romdis_in) return;   // no change
	romdis_in = f;
	if(f) return;

	cpu->mapRom(0x0000,0x4000,&rom[0],NULL,0);
}*/

uint8 CurrahMicroSpeech::readMemory(Time t, int32 cc, uint16 addr, uint8 byte)
{
	// handle memory mapped i/o:
	// 0x0038: toggle enable state
	// 0x1000: read state

	// toggle enable state:
	if(addr==0x0038)
	{
		xlogIn("CurrahMicroSpeech::readMemory(0x0038)");
		toggle_enable_state();
		return machine->cpu->peek(addr);	// re-read byte from new rom
	}

	// read SP0256 busy status:
	// probably LRQ (load request), not SBY (stand-by)
	// bits 1..7 probably floating
	if(enable_state && (addr & READ_STATE_MASK) == READ_STATE_ADDRESS)
	{
		bool busy = sp0256->isBusy(t);
		xxlogIn("CurrahMicroSpeech::readMemory(0x%04x) --> sp0256 is %s",addr,busy?"busy":"not busy");
		return busy ? byte | 0x01 : byte & 0xFE;
	}

	return prev()->readMemory(t,cc,addr,byte);
}

void CurrahMicroSpeech::writeMemory( Time t, int32 cc, uint16 addr, uint8 byte )
{
	// handle memory mapped i/o:
	// 0x0038: toggle enable state
	// 0x1000: write sp0256 command
	// 0x3000: set intonation (pitch). pitch changes slowly: 0.1 ~ 0.5s

	// toggle enable state:
	if(addr==0x0038)
	{
		logIn("CurrahMicroSpeech::writeMemory(0x0038)");
		toggle_enable_state();
		return ;
	}

	// write sp0256 command:
	if(enable_state && (addr & WRITE_COMMAND_MASK) == WRITE_COMMAND_ADDRESS)
	{
		if(XXLOG || (XLOG && byte!=0))
			logIn("CurrahMicroSpeech::writeMemory: 0x%04x = 0x%02x --> command",addr,byte);

		byte &= 0x3F;			// bits[0…5] = command
		sp0256->writeCommand(t,byte);
		add_history(byte);
	}

	// set pitch:
	else if(enable_state && (addr & SET_INTONATION_MASK) == SET_INTONATION_ADDRESS)
	{
		if(addr & 1)
		{
			xxlogIn("CurrahMicroSpeech:writeMemory(0x%04x) --> set intonation HIGH",addr);
			pitch = 0x40;
			clock = clock_high;
		}
		else
		{
			xxlogIn("CurrahMicroSpeech:writeMemory(0x%04x) --> set intonation LOW",addr);
			pitch = 0x00;
			clock = clock_low;
		}
	}

	prev()->writeMemory(t,cc,addr,byte);
}

void CurrahMicroSpeech::setHifi(bool f) volatile
{
	//	set sp0256 emulation to normal or "HiFi" mode:
	//	actually this makes no audible difference

	assert(isMainThread());

	sp0256->setHifi(f);
}

bool CurrahMicroSpeech::isHifi() volatile const
{
	return sp0256->isHifi();
}

void CurrahMicroSpeech::add_history(uint8 command)
{
	// add command to the history ring buffer for display in the inspector:
	// 0x00 .. 0x3F	allophone
	//
	// --> in history[]:
	// 0x05 .. 0x3F	allophone pitch_low
	// 0x45 .. 0x7F	allophone pitch_high
	// 0x80			reserved for illegal command (can't happen)
	// 0x81 .. 0xFF	pause with N-0x80 repetitions -> (N-0x80)*64 samples @ 10kHz
	// --> pause:
	// if pause>0	then we are accumulating pause after the last entry in history[]
	// 			either for pause commands played or for the SP0256 going into stand_by
	// 			this will be stored as a pause in history[] before the next real allophone played

	const uint mask = NELEM(history) - 1;

	if(command<5)	// accumulate pause
	{
		static const uint dur[] = {1,4,7,15,31};	// repetitions in pause opcode
		pause += dur[command];
		return;
	}

	if(pause)		// store accumulated pause into history[]
	{
		history[lastwp & mask] = min(0xFFu, 0x80+pause);
		pause = 0;	// tiny race condition here
		++lastwp;
	}

	// store allophone
	history[lastwp & mask] = command + pitch;
	++lastwp;
}

void CurrahMicroSpeech::saveToFile(FD&) const noexcept(false) /*file_error,bad_alloc*/{TODO();}
void CurrahMicroSpeech::loadFromFile(FD&) noexcept(false) /*file_error,bad_alloc*/{TODO();}














