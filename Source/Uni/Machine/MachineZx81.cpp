/*	Copyright  (c)	Günter Woigk 2008 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "MachineZx81.h"
#include "ZxInfo.h"
#include "Z80/Z80.h"
#include "unix/FD.h"
#include "MachineController.h"
#include "TapeRecorder.h"
#include "Ram/Memotech64kRam.h"
#include "Ram/Zx16kRam.h"
#include "Audio/O80Data.h"
#include "Keyboard.h"


// how much space must be left free in addition to the program loaded?
// note: a ZX81 tape file contains the screen file,
// but it's probably collapsed if ram size <= 4 kB
//
#define MIN_FREE_16k	(80)		// edit line + some spare bytes...
#define MIN_FREE_4k	    (24*32+80)	// full screen - collapsed screen + edit line + some spare bytes...

static const uint16 BREAK_CONT_REPEATS	= 0x03A6;   // BREAK - CONT repeats
static const uint16 SLOW_FAST			= 0x0207;	// SLOW_FAST


inline int progname_len( cu8ptr p, int n )
{
	// helper: saved data starts with a program name, 127 char max and last char | 0x80

	if(n>127) n=127;
	for( int i=0; i<n; i++ ) { if( p[i]&0x80 ) return i+1; }
	return n;
}

MachineZx81::MachineZx81(MachineController* m, isa_id id, Model model)
:
	Machine(m,model,id)
{
	audio_in_enabled = no;	// default. MachineController will override if flag set in settings
}

MachineZx81::MachineZx81(MachineController* m)
:
	Machine(m,zx81,isa_MachineZx81)
{
	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaZx81(this);	// should be 2nd item
	mmu			= new MmuZx81(this);
	keyboard	= new KeyboardZx81(this);
	//ay		=
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);

	audio_in_enabled = no;	// default. MachineController will override if flag set in settings
}

bool MachineZx81::handleSaveTapePatch()
{
	// ROM PATCH: save tape block
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	xlogIn("MachineZx81:handleSaveTapePatch");

	// test whether the tape recorder can record a block:
	if(!taperecorder->can_store_block()) return 0;		 // not handled

	Z80Regs& regs = cpu->getRegisters();
	assert(regs.pc==0x02FC);				// verify patch address

	uint dataend = cpu->peek2(0x4014);
	uint datalen = dataend - 0x4009;

// sanity test:
	if(dataend < 0x403c || dataend > 0x4000+ram.count())
	{
		showWarning("Illegal sysvar E_LINE ($4014): %u\nThe programme was NOT saved!", uint(dataend));
		regs.pc = BREAK_CONT_REPEATS;
		return 1;		// handled
	}

// get program name:
// (hl)++ until bit 7 set
	uint8 pname[127];
	uint  pnamelen = 0;
	uint  hl = regs.hl;
	while( pnamelen < 127 && (pname[pnamelen++] = cpu->peek(hl++)) < 0x80 ){}

// get data
	Array<uint8> data(datalen+pnamelen);
	memcpy(&data[0],pname,pnamelen);
	cpu->copyRamToBuffer(0x4009,&data[pnamelen],datalen);

// write to tape:
	taperecorder->storeBlock(new O80Data(std::move(data),yes/*zx81*/));

// set registers:
	regs.pc = SLOW_FAST;
	regs.bc = 0x0080;
	regs.de = 0xffff;
	//reg.iy = 0x4000;
	//reg.im = 1;
	//reg.i  = 0x1e;
	//reg.iff1 = reg.iff2 = 0;

	return 1;   // handled
}

bool MachineZx81::handleLoadTapePatch()
{
	// ROM PATCH: load tape block
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	xlogIn("MachineZx81:handleSaveTapePatch");

// test whether the tape recorder can read a block:
	if(!taperecorder->can_read_block()) return 0;		 // not handled

	Z80Regs& regs = cpu->getRegisters();
	if(regs.pc != 0x0347) return 0;		// patch address = NEXT-PROG

a:	O80Data* bu = taperecorder->getZx80Block();
	if(!bu) return 0;					// not handled
	uint32 sz = bu->count(); uint8* data = bu->getData();
	assert(sz>0);
	assert(bu->trust_level >= TapeData::truncated_data_error);

// compare program name (DE++)
// the ZX81 compares the first bytes from tape against the desired program name
// and resumes loading if it matches, else skips this block and tries the next.
// we could bail out here too and jump to the retry PC position
// or we simply resume with the next block too:
	uint   l  = progname_len(data,sz);
	uint16 de = regs.de;		// de -> program name
	if((de&0x8000)==0)			// except if d.bit7==1
		// && data[0]!=0x80 )	// except if file has no filename stored: .p and .81 files
	{
		for(uint i=0;i<l;i++) { if(cpu->peek(de+i)!=data[i]) goto a; }
	}
	data += l; sz -= l;			// omit prog name

// calculate length of actually loaded data:
	uint end = sz<=0x4015-0x4009 ? 0x4009 + sz : peek2Z(data+0x4014-0x4009);
	uint len = min(sz,max(end,0x4015u)-0x4009u);

// copy data to ram:
	cpu->copyBufferToRam(data,0x4009,len);

// detect errors:
	if(bu->isZX80())
	{
		regs.pc = BREAK_CONT_REPEATS;
		showWarning("Programme is for a ZX80");
		return 1;
	}
	if(end<0x403c)
	{
		regs.pc = BREAK_CONT_REPEATS;
		showWarning("Data corrupted: data is too short: len < sysvars");
		return 1;
	}
	if(0x4009+len<end)
	{
		regs.pc = BREAK_CONT_REPEATS;
		showWarning("Data corrupted: data is too short: len < ($4014)-$4009");
		return 1;
	}
	if(end>0x4000+ram.count())
	{
		showWarning("Programme did not fit in ram.\nProgramme size = %u bytes", uint(end-0x4000));
	}
	else if(end>regs.sp)
	{
		showInfo("Note: The machine stack was overwritten by the data");
	}
	else if(end+256>0x4000+ram.count())
	{
		showWarning("Programme uses almost all ram and may require more ram to run.");
	}

// ok: set registers:
	regs.pc = SLOW_FAST;
	regs.bc = 0x0080;
	regs.de = 0xffff;
	return 1;
}

void MachineZx81::saveP81(FD &fd, bool p81) noexcept(false) /*file_error,data_error*/
{
	// SNAPSHOT: save a ZX81 .p, .81 or .p81 file:
	// data contains all ram from $4009 to ($4014)  (sysvar E_LINE)
	// note:
	//   Video memory IS included in ZX81 files.
	//   the last byte of a (clean) file should be $80 (the last byte of VARS)
	// note:
	//   .p / .81 / .p81 is a tapefile. saving as snapshot will only work
	//   if saved while the ZX81 is waiting at the command prompt!

	uint end = cpu->peek2(0x4014);
	if( end<0x403c ) throw data_error("Save Tape: illegal sysvar E_LINE ($4014): %u",end);

// get data
	uint16 cnt = end - 0x4009u;
	uint8 data[1+cnt];
	data[0]=0x80;	// empty name if .p81
	cpu->copyRamToBuffer(0x4009,data+1,cnt);

// write to file:
	if(p81) fd.write_bytes(data,cnt+1);
	else fd.write_bytes(data+1,cnt);
}

void MachineZx81::loadP81(FD &fd, bool p81) noexcept(false) /*file_error,data_error*/
{
	// SNAPSHOT: load a ZX80 .p, .81 or .p81 file:
	// loads data into ram from address $4009 to ($4014)		(sysvar E_LINE)
	// sets PC as after rom tape load routine
	// detaches an existing ram extension if too small
	// and attaches a ram extension if required
	//   possible ram extensions are:
	//   - zx16kram
	//   - memotech64kram
	// --> machine is powered up but suspended

	xlogIn("MachineZx81:loadP81(fd)");

	assert(is_locked());

// skip program name:
	uint pnamelen=0;
	if(p81) while( ++pnamelen <= 127 && fd.read_uint8() < 0x80 ){}

// get actual data size:
	uint len = fd.file_remaining();
	if(len>0x15-0x09)
	{
		fd.skip_bytes(0x14-0x09);
		uint end = fd.read_uint16_z();
		fd.skip_bytes(-(2+0x14-0x09));
		if(end>0x4015) len = min(len,end-0x4009);
	}

// attach external ram if required
// note: MachineController must update the menu entries!
	if(len+MIN_FREE_16k>ram.count() && len+MIN_FREE_16k>16 kB)
	{
		delete findIsaItem(isa_ExternalRam);
		new Memotech64kRam(this);
	}
	else if(ram.count()<16 kB && len+MIN_FREE_4k>ram.count())
	{
		delete findIsaItem(isa_ExternalRam);
		new Zx16kRam(this);
	}

	// we need to power on the machine but it must not runForSound()
	// don't block: we might be called from runForSound()!
	_suspend();
	_power_on();

	uint ramsize = min(0xBFFEu,ram.count());

// nicht überschriebene Systemvariablen initialisieren:

	cpu->poke (0x4000,0xff);		  // ERR_NR  Errorcode-1
	cpu->poke (0x4001,0x80);		  // FLAGS   Various BASIC Control flags: Bit1=Redirect Output to printer
	cpu->poke2(0x4002,0x3ffc+ramsize);// ERR_SP  Pointer to top of Machine Stack / Bottom of GOSUB Stack
	cpu->poke2(0x4004,0x4000+ramsize);// RAMTOP  Pointer to unused/free memory (Changes realized at next NEW or CLS)
	cpu->poke (0x4006,0x00);		  // Selects [K], [L], [F], or [G] Cursor
	cpu->poke2(0x4007,0xfffe);		  // PPC     Line Number of most recently executed BASIC line  (($FFFE=cmd line))

// setup registers for 'success':
// Aussprungstelle der "Alle Bytes geladen?" Testroutine:
// version 2 'improved' rom:
	Z80Regs& regs = cpu->getRegisters();

	regs.pc = SLOW_FAST;
	regs.sp = 0x4000+ramsize;
	cpu->push2(0x3e00);			// always
	cpu->push2(0x0676);			// always
	regs.bc = 0x0080;			// always
	regs.de = 0xffff;			// always
	regs.ix = 0x0281;			// TODO: nötig?
	regs.iy = 0x4000;			// must be
	regs.de2 = 0x002b;			// TODO: nötig?
	regs.im = 1;				// must be
	regs.i  = 0x1e;				// must be
	regs.iff1 =					// must be
	regs.iff2 = disabled;		// must be

// load data:
	uint8 data[len];
	fd.read_bytes(data,len);	// throws
	cpu->copyBufferToRam(data,0x4009,len);

// show possible issues:
	if(len<0x3c)
	{
		regs.pc = BREAK_CONT_REPEATS;
		showWarning("Data corrupted: data is too short: len < sysvars");
	}
	else if(0x4009+len<cpu->peek2(0x4014))
	{
		regs.pc = BREAK_CONT_REPEATS;
		showWarning("Data corrupted: data is too short: len < ($4014)-$4009");
	}
	else if(0x4009+len>cpu->getRegisters().sp)
	{
		showInfo("Note: The machine stack was overwritten by the data");
	}
}


/*	Notes:

	ZX81 Cassette File Structure
	----------------------------

		x seconds    your voice, saying "filename" (optional)
		x seconds    video noise
		5 seconds    silence
		1-127 bytes  filename (bit7 set in last char)
		LEN bytes    data, loaded to address 4009h, LEN=(4014h)-4009h.
		1 pulse      video retrace signal (only if display was enabled)
		x seconds    silence / video noise

		The data contains system area, basic program, video memory, VARS.
		the last byte of a (clean) file should be $80 (the last byte of VARS)

		$4014	defines the end address (used to calculate the file length)
		$4029	points to the next executed (autostarted) BASIC line
		$403B	indicates if program runs in SLOW or FAST mode (bit 6)
		$403C++	may be misused for whatever purpose,
		video memory must contain 25 HALT opcodes if the file was saved in SLOW mode.

		Files should usually not exceed 16 kBytes.
		The memory detection procedure in both ZX80 and ZX81 BIOS stops after 16 kBytes (at $8000),
		and initializes the stack pointer at that address, even if more memory is installed.
		Thus loading files of 16k or more would destroy the stack area,
		unless a separate loader has previously moved the stack area to another location.
		However, most ZXes don't have more than 16k RAM, so bigger files won't work on most computers anyways.

		.81 and .p files:	include only the data, loaded to $4009++
		.p files:			typically there is some garbage at the file end
		.p81 files:			start with the 1..127 bytes filename, last byte ORed with $80


	LOAD TAPE Routine
	-----------------

		in:		DE -> fname				except if D.bit7=1  =>  load any file
		Patch is placed at 0x0347.

	L0340:  CALL    L03A8           ; routine NAME  ->  DE points to start of name in RAM.
			RL      D               ; pick up carry
			RRC     D               ; carry now in bit 7.
	;; NEXT-PROG
	L0347:  CALL    L034C           ; routine IN-BYTE		<-- patch must be applied here
			JR      L0347           ; loop to NEXT-PROG

	Registers on success:

		Aussprungstelle der "Alle Bytes geladen?" Testroutine:
		version 2 'improved' rom:

	*   regs.pc = 0x0207;
		regs.sp = 0x4000+ramsize;   preserved
		cpu.push2(0x3e00);
		cpu.push2(0x0676);
		reg.af =					egal: will be overwritten in $020A
	*	regs.bc = 0x0080;
	*	regs.de = 0xffff;
		reg.hl =					egal: will be overwritten in $0207
		reg.ix =					preserved
		reg.iy = 0x4000;			preserved
		reg.af2 = 0x0000 or 4e81	egal
		reg.bc2 =					preserved
		reg.de2 = 0x002b;			preserved
		reg.hl2 =					preserved
		reg.im = 1;                 preserved
		reg.i  = 0x1e;              preserved
		reg.r  = 0xa6;              egal
		reg.iff1 = reg.iff2 = 0;	preserved


	SAVE TAPE Routine
	-----------------

		'SAVE' command routine address: 0x02F6
				0x02F6:	parse cmd line for fname
			=>	the patch must be applied at 0x02FC.
				0x02FF:	test for break				// first possible address for patching
				0x0304:	5 seconds delay				// <-- suggested address for patching
				0x030B:	save name: "myname"+$80		// last possible address for patching
				0x0316:	save data
		saves fname:
				(hl)++ until bit 7 set
		saves memory:
				base addr = 0x4009
				data size = (0x4014) - 0x4009

	Registers on success:

		regs.pc = 0x0207;			SLOW_FAST
		reg.sp =					no change
		reg.af =					overwritten in $020A
		regs.bc = 0x0080;			always
		regs.de = 0xffff;			always
		reg.hl =					overwritten in $0207

		reg.ix =					no change
		reg.iy = 0x4000;			no change

		reg.af2 = 0x0000 or 4e81	don't care
		reg.bc2 =					no change
		reg.de2 = 0x002b;			no change
		reg.hl2 =					no change

		reg.im = 1;					no change
		reg.i  = 0x1e;				no change
		reg.r  = 0xa6;				arbitrary
		reg.iff1 = reg.iff2 = 0;	no change
*/














