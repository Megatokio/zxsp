/*	Copyright  (c)	Günter Woigk 2008 - 2019
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

#include "MachineZx80.h"
#include "ZxInfo.h"
#include "Z80/Z80.h"
#include "unix/FD.h"
#include "TapeRecorder.h"
#include "MachineController.h"
#include "Items/Ram/Memotech64kRam.h"
#include "Items/Ram/Zx3kRam.h"
#include "Items/Ram/Zx16kRam.h"
#include "Audio/O80Data.h"
#include "Settings.h"


// how much space must be left free in addition to the program loaded?
// note: a ZX80 tape file does not contain the screen file
//
#define MIN_FREE	(24*33+32)		// full screen + edit line + some spare bytes...


MachineZx80::MachineZx80(MachineController* parent, Model model, isa_id id)
:
	Machine(parent,model,id)
{
	cpu			= new Z80(this);
	ula			= new UlaZx80(this); ula->set60Hz(settings.get_bool(key_framerate_zx80_60hz,false));
	mmu			= new MmuZx80(this);
	keyboard	= new KeyboardZx80(this);
	//ay		=
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);

	audio_in_enabled = no;	// default. MachineController will override if flag set in settings
}

bool MachineZx80::handleSaveTapePatch()
{
	// handle rom patch
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	xlogIn("MachineZx80:handleSaveTapePatch");

// test whether the tape recorder can record a block:
	if(!taperecorder->can_store_block()) return 0;		 // not handled

// set registers:
	cpu->getRegisters().pc = 0x0283;	// MAIN_EXEC

	uint dataend = cpu->peek2(0x400A);
	uint datalen = dataend - 0x4000;

// sanity test:
	if(datalen<0x28 || datalen>ram.count())
	{
		showWarning("Illegal sysvar E_LINE ($400A): %u\nThe programme was NOT saved!", uint(dataend));
		return 1;		// handled
	}

// store data
	Array<uint8> buffer(datalen);
	cpu->copyRamToBuffer(0x4000,&buffer[0],datalen);
	taperecorder->storeBlock(new O80Data(std::move(buffer),no/*!zx81*/));

	return 1;   // handled
}

bool MachineZx80::handleLoadTapePatch()
{
	// handle rom patch
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	xlogIn("MachineZx80:handleLoadTapePatch");

// test whether the tape recorder can read a block:
	if(!taperecorder->can_read_block()) return 0;		 // not handled

// get data from tape:
	O80Data* bu = taperecorder->getZx80Block();
	if(!bu) return 0;					// end of file	=> 0 = not handled
	assert(bu->trust_level >= TapeData::conversion_success);
	uint32 sz = bu->count(); uint8* data = bu->getData();
	assert(sz>0);

// store data in ram:
	uint len = sz<11 ? sz : min( sz, max(0x400Au, uint(peek2Z(data+0x0A)))-0x4000u );
	cpu->copyBufferToRam(data,0x4000,len);

// set registers:
	cpu->getRegisters().pc = 0x0283;	// pc: MAIN_EXEC

// show possible issues:
	if(bu->isZX81())						 showWarning("Programme is for a ZX81");
	else if(len<0x28)						 showWarning("Data corrupted: data is too short: len < sysvars");
	else if(0x4000+len<cpu->peek2(0x400A))	 showWarning("Data corrupted: data is too short: len < ($400A)-$4000");
	else if(len>ram.count()-25/*min.screen*/)showWarning("Programme did not fit in ram.\nProgramme size = %u bytes",uint(len));
	//else if(len>ram.count()-MIN_FREE)		 showWarning("Programme uses almost all ram and may require more ram to run.");
	else if(0x4000+len>cpu->getRegisters().sp) showInfo("Note: The machine stack was overwritten by the data");

	return 1;							// handled
}

void MachineZx80::saveO80(FD &fd) throws
{
	// save a SNAPSHOT: save a ZX80 .o or .80 file:
	// data contains all ram from $4000 to ($400A)		(sysvar E_LINE)
	// note:
	// 	 Video memory is NOT included in ZX80 files.
	// 	 the last byte of a (clean) file should be $80 (the last byte of VARS)
	// note:
	// 	 .o / .80 is a tapefile. saving as snapshot will only work
	// 	 if saved while the ZX80 is waiting at the command prompt!

	xlogIn("MachineZx80:saveO80(fd)");

	uint16 len = cpu->peek2(0x400A)-0x4000u;
	uint8  bu[len];
	cpu->copyRamToBuffer(0x4000,bu,len);
	fd.write_bytes(bu,len);
}

void MachineZx80::loadO80(FD &fd) noexcept(false) /*file_error,data_error*/
{
	// load a SNAPSHOT: load a ZX80 .o or .80 file:
	// loads data into ram from address $4000 to ($400A)		(sysvar E_LINE)
	// sets PC as after rom tape load routine
	// detaches an existing ram extension if too small
	// and attaches a ram extension if required
	//   possible ram extensions are:
	//   - zx3kram
	//   - zx16kram
	//   - memotech64kram
	// --> machine is powered up but suspended

	xlogIn("MachineZx80:loadO80(fd)");

	assert(is_locked());

// get actual data size:
	uint len = fd.file_size();
	if(len>=11)
	{
		fd.seek_fpos(10);
		len = min( len, max(0x400Au, uint(fd.read_uint16_z()))-0x4000u );
		fd.rewind_file();
	}

// attach external ram if required
// note: MachineController must update the menu entries!
	uint req_len = len + MIN_FREE;
	if(req_len>ram.count())
	{
		delete findIsaItem(isa_ExternalRam);

		if(req_len>16 kB)     new Memotech64kRam(this);		// note: required a small HW patch to work with the ZX80
		else if(req_len>4 kB) new Zx16kRam(this);
		else				  new Zx3kRam(this,(req_len-1)/0x4000*0x4000); // 1 .. 3 kB
	}

	// we need to power on the machine but it must not runForSound()
	// don't block: we might be called from runForSound()!
	_suspend();
	_power_on();

// set registers:
	Z80Regs& regs = cpu->getRegisters();
	regs.pc = 0x0283;				// MAIN_EXEC
	regs.sp = 0x4000+min(ram.count(),0xC000u);
			  cpu->push2(0x3f82);	// TODO: what is 3f82? ((823f??)) seems to be never used for ret.
	regs.iy = 0x4000;               // must be 4000
	regs.im = 1;                    // must be 1
	regs.i  = 0x0e;                 // must be (character set)

// load data:
	uint8 data[len];
	fd.read_bytes(data,len);
	cpu->copyBufferToRam(data,0x4000,len);

// show possible issues:
	if(len<0x28)							showWarning("Data corrupted: data is too short: len < sysvars");
	else if(0x4000+len<cpu->peek2(0x400A))	showWarning("Data corrupted: data is too short: len < ($400A)-$4000");
	else if(0x4000+len>cpu->getRegisters().sp) showInfo("Note: The machine stack was overwritten by the data");
}


/*	-------------------------------------------------------------------------

	INFO

	ZX80 Cassette File Structure
		x seconds    your voice, saying "filename" (optional)
		x seconds    video noise
		5 seconds    silence
		LEN bytes    data, loaded to address 4000h, LEN=(400Ah)-4000h.
		x seconds    silence / video noise

		ZX80 files do not have filenames
		ZX80 files cannot be autostarted.
		The data is loaded to address $4000++
		The data contains the whole system area, basic program, VARS.
		Video memory is NOT included in ZX80 files.
		the last byte of a (clean) file should be $80 (the last byte of VARS)
		The system area should contain proper data.
		$400A	defines the end address (used to calculate the file length).
		$4028++	may be misused for whatever purpose.

	.80 and .o files:	include only the data, loaded to $4000++
	.o files:			typically there is some garbage at the file end


	The patch is placed on address 0x0207.

Registers after successful loading:

		Registers PC, IY, IM, I and IFF1 and IFF2 must be set if loading a snapshot.
		Otherwise they are registers out == registers in.

	regs.pc = 0x0283;       	// MAIN_EXEC
	regs.sp = 0x4000+ramsize;
	cpu->push2(0x3f82);         // TODO: what is 3f82? seems to be never used for ret.  PROBABLY $823f (screen refresh) ?

	regs.af = 0x3583;
	regs.bc = 0x0035;           //	overwritten in $0747 CLS
	regs.de = 0x0000;           //	or ffff or 4046 or 0000 or 000a
	regs.hl = 0x12C3;           //	overwritten immediately in MAIN_EXEC

	regs.ix = 0x0000;           // seemingly always 0000
	regs.iy = 0x4000;           // must be 4000

	regs.af2 = 0x0000;          // seemingly always 0000
	regs.bc2 = 0x0021;          // or 1721 or fff6
	regs.de2 = 0xd8f0;          // or 401c
	regs.hl2 = 0xd8f0;          // or 402b or 048b

	regs.im   = 1;              // must be 1
	regs.i    = 0x0e;           // must be (character set)
	regs.iff1 =                 // must be (A6 in refresh cycle triggers interrupt)
	regs.iff2 = disabled;
*/












