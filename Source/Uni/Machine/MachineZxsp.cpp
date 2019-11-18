/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "MachineZxsp.h"
#include "TapeRecorder.h"
#include "Z80/Z80.h"
#include "Z80/Z80opcodes.h"
#include "IsaObject.h"
#include "Ula/UlaZxsp.h"
#include "Audio/TapData.h"
#include "SpectraVideo.h"
#include "Keyboard.h"


MachineZxsp::MachineZxsp(MachineController*m, Model model, isa_id id)
:	Machine(m,model,id){}

MachineZxsp::MachineZxsp(MachineController*m, Model model)
:
	Machine(m,model,isa_MachineZxsp)
{
	assert(model==zxsp_i1 || model==zxsp_i2 || model==zxsp_i3 || model==zxplus);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaZxsp(this);	// should be 2nd item
	mmu			= new MmuZxsp(this);
	keyboard	= model==zxplus ? static_cast<Keyboard*>(new KeyboardZxPlus(this)) : new KeyboardZxsp(this);
	//ay		=
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}

static uint8 calc_zxsp_tapeblock_crc ( uint8 const* data, int cnt, uint8 crc ) noexcept
{
	for( int i=0; i<cnt; i++ ) { crc ^= data[i]; }
	return crc;
}

bool MachineZxsp::handleSaveTapePatch()
{
	// handle rom patch
	// called with all z80 registers stored
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	// save data as with the ZXSP ROM 'save tape' routine
	// nominal routine address: 0x04D0
	// • addr = IX
	// • size = DE
	// • pilot length = HL
	// • type byte = A
	// • return address = (SP++)
	// return:	registers set for 'ok'

	xlogIn("MachineZxsp::handleSaveTapePatch");

	// test whether the tape recorder can record a block:
	if(!taperecorder->can_store_block()) return 0;		 // not handled

	Z80Regs& regs = cpu->getRegisters();
	uint16 addr  = regs.ix;	// data address
	uint16 cnt   = regs.de;	// data size
	uint16 pilot = regs.hl;	// pilot pulses
	uint8  a	 = regs.a;	// type byte

// preset to 'breaked':
	//regs.ix++;  			// first 'break' test is after first saved byte
	//regs.de--;
	//regs.a = 0xFE>>1;		// RRA:	a = (a>>1) + (f<<7)
	//regs.f = 0x00;		//		f = (f&~(C_FLAG+N_FLAG+H_FLAG))
	//regs.pc = cpu->pop2();// RET NC

// store data:
	Array<uint8> data(cnt+2);
	data[0] = a;
	cpu->copyRamToBuffer(addr,&data[1],cnt);
	data[cnt+1] = calc_zxsp_tapeblock_crc(&data[1],cnt,a);
	taperecorder->storeBlock(new TapData(data,pilot,1.0,yes));

// return 'ok':
	regs.ix += cnt;			// += regs.de+1
	regs.de = 0xFFFF;		// -= regs.de+1
	regs.a	= 0;			//			after jp nz,BYTEAU	; $04FE
	regs.b  = 0;			//			after DJNZ
	regs.f  = C_FLAG;		// 'ok'		after jp nz,BYTEAU	; $04FE
	regs.pc = cpu->pop2();	// RET C

	return 1;	// handled
}

bool MachineZxsp::handleLoadTapePatch()
{
	xlogIn("MachineZxsp::handleLoadTapePatch");

// test whether the tape recorder can read a block:
	if(!taperecorder->can_read_block()) return 0;		 // not handled

	TapData* bu = taperecorder->getZxspBlock();
	if(!bu) return 0;		// not handled

/*	load data from tape file as with the ZX Spectrum Rom load routine
	nominal patch address: 0x0556
	also works for: 32K rom and 64K rom variants
		(same cpu address, different offset in rom image)
	also works for: TC2068 variants
		(different cpu address but same offsets inside the tape routine, different offset in rom image)

	in:
		IX:	addr
		DE:	count		(D must not be $FF)
		A:	blocktype	(byte 1)
		F:	CY = load
			NC = verify
	ok:
		CY,NZ = ok
		A	  = 0 (crc)
		B	  = 0xB0
		C     = 0x02 oder 0x22  je nach letztem ear_in bit
		H     = 0 (crc)
		L     = last (crc) byte
		IX	 += count
		DE	  = 0		( -= count)
		AF,BC,DE,HL,AF',IX modified
		IFF	disabled	(will be enabled in SAVLOA)
		PC	return address popped from stack
		• SAVLOA ($053F) wir anfangs auf den Stack gepusht und kann dort überschrieben werden
		• FLANK2 ($05E3) und darin FLANK1 ($05E7) werden für jedes Bit aufgerufen und überschreiben die geladenen Daten
	err:
		NC    = break or error
		NC NZ = break						IX,DE entspr. geladenen bytes
		NC NZ = wrong block type			IX,DE untouched;  EX AF,AF';  L = block type found
		NC NZ = verify error				IX,DE entspr. wrong byte;  EX AF,AF';
		NC Z  = error (pulse too long)
		NC Z  = final CRC == 1				IX,DE entspr. count;  A = H = 1;  L = last (crc) byte
		NC NZ = final CRC wrong (but not 1)	IX,DE entspr. count;  A = H > 1;  L = last (crc) byte
*/

	cu8ptr q = bu->getData();
	uint32 qlen = bu->count();

	Z80Regs& regs   = cpu->getRegisters();
	uint16   zaddr  = regs.ix;				// target address
	uint32   zlen   = regs.de +2;			// target data size incl. block_type and crc
	uint8    ztype  = regs.a;				// target block type
	bool     verify = !(regs.f & C_FLAG);	// flag: verify data, else load data
	bool	 trunc  = qlen<zlen;            // flag: truncated, source data too short
	qlen = min(qlen,zlen);					// 2013-09-14

// preset for 'error' if we throws:
// • wie nach FLANK2 test failed:
// • IX, DE untouched (no bytes stored/verified)
	cuint16 SAVLOA    = model_info->tape_load_ret_addr & 0xfff;
												// 0x053F;	return address pushed on stack at start of routine
	cuint16 FLANK2RET = SAVLOA + 0x05CD-0x053F;	// 0x05CD;	ret addr on stack when calling FLANK2
	cuint16 FLANK1RET = SAVLOA + 0x05E6-0x053F;	// 0x05E6;	ret addr on stack when calling FLANK1
	cpu->push2(SAVLOA);
	cpu->push2(FLANK2RET);
	cpu->push2(FLANK1RET);
	regs.sp += 6;					// undo pushes
	regs.pc = SAVLOA;				// ~ RET
	regs.iff1 = regs.iff2 = disabled;
	regs.c = 0x22;					// ear_in bit
	regs.f = Z_FLAG;				// NC & Z

// exclude block type and crc from buffers:
	uint8 qtype = *q;				// block type on tape
	q++; qlen--; zlen--;			// skip type byte
	if(!trunc) { qlen--; zlen--; }	// if not truncated then exclude the crc byte from buffer data

// check block type:
	if( qtype != ztype )
	{
		regs.l = qtype;				// block type found
		regs.a = qtype ^ ztype;
		regs.f = 0x00;				// NC,NZ  ->  error
		return 1;					// handled
	}

// load or verify bytes:
	if( verify )					// verify
	{
		uint8 zbu[qlen];
		uint8* z = &zbu[0];
		cpu->copyRamToBuffer(zaddr,z,qlen);
		for( long i=0; i<qlen; i++ )
		{
			if( q[i] != z[i] )
			{
				regs.ix+=i;
				regs.de-=i;
				regs.f=0x00;
				return 1;			// handled - verify error
			}
		}
	}
	else							// load
	{
		cpu->copyBufferToRam(q,zaddr,qlen);
		regs.sp -= 2;				// redo PUSH SAVLOA which may be overwritten
		cpu->push2(FLANK2RET);		// redo
		cpu->push2(FLANK1RET);		// redo
		regs.sp += 4;				// undo 2x pushes
		regs.pc = cpu->pop2();		// redo RET to SAVLOA, hopefully
	}

// check load error: block on tape too short
	if( trunc )
	{
		regs.ix += qlen;
		regs.de -= qlen;
		regs.f = Z_FLAG;			// NC,Z: pulse too long
		return 1;					// handled
	}

// ok or crc error:
	uint8 crc = calc_zxsp_tapeblock_crc(q,qlen,qtype);	// calculated crc for qtype + qdata
	regs.h = regs.a = crc ^= regs.l = q[qlen];			// actual crc on tape;  final XOR must result in 0x00
	regs.ix += qlen;
	regs.de  = 0;
	regs.b = 0xB0;
	regs.f = crc==0 ? C_FLAG : crc==1 ? Z_FLAG : 0x00;	// CY,NZ = ok; else crc error

	return 1;		// handled
}

void MachineZxsp::saveScr(FD& fd) throws
{
	// save a ZX Spectrum .scr screenshot
	// this is the standard 6912 byte frame buffer of a ZX Spectrum
	// ZX128: the actually visible frame buffer is saved.

	uint8 bu[6912];
	Z80::c2b(UlaZxspPtr(ula)->getVideoRam(),bu,6912);
	fd.write_bytes(bu,6912);
}

void MachineZxsp::loadScr(FD& fd) throws
{
	uint32 sz = uint32(fd.file_size());
	if(sz!=6912) showWarning("The .scr Screen file seems to be corrupted");
	sz = min(6912u,sz);
	uint8 bu[6912];
	fd.read_bytes(bu,sz);

	if(crtc->isA(isa_SpectraVideo))
		SpectraVideoPtr(crtc)->setVideoMode(SpectraVideoPtr(crtc)->getVideoMode() & 0x60);

	CoreByte* videoram = crtc->getVideoRam();
	Z80::b2c(bu,videoram,sz);

	uint8 a = (videoram[32*24*8   ] >> 3) & 7;
	uint8 b = (videoram[32*24*8+31] >> 3) & 7;
	uint8 c = (videoram[32*24*9-32] >> 3) & 7;
	uint8 d = (videoram[32*24*9-1 ] >> 3) & 7;

	crtc->setBorderColor(c==d ? c : a==b ? a : d==a || d==b ? d : c);
}

#define snalen 27

void write_mem(FD& fd, const CoreByte *q, uint32 cnt) throws
{
	std::unique_ptr<uint8[]> bu{new uint8[cnt]};
	Z80::c2b(q,bu.get(),cnt);
	fd.write_bytes(bu.get(),cnt);
}

void read_mem(FD& fd, CoreByte* z, uint32 cnt) throws
{
	std::unique_ptr<uint8[]> bu{new uint8[cnt]};
	fd.read_bytes(bu.get(),cnt);
	Z80::b2c(bu.get(),z,cnt);			// copy data, preserve flags
}

Model modelForSna(FD& fd) throws
{
	uint32 ramsize = uint32(fd.file_size())-snalen;
	return ramsize > 0x4000 ? zxsp_i3 : zxsp_i1;
}

struct SnaHead
{
	uint8
		i,
		l2,h2,e2,d2,c2,b2,f2,a2,
		l,h,e,d,c,b,
		yl,yh,xl,xh,
		iff,r,f,a,spl,sph,im,
		brdr;

	void    clear()							{ memset(this,0,snalen); }
	void    readFromFile(FD& fd)   throws		{ fd.read_bytes(this,snalen); }
	void    writeToFile(FD& fd)    throws		{ fd.write_bytes(this,snalen); }

	void    setRegisters(Z80Regs const&);	// put regs into SnaHead.   ATTN: push PC!
	void    getRegisters(Z80Regs&);			// get regs from SnaHead.   ATTN: pop PC!
};

void SnaHead::setRegisters(Z80Regs const& regs)
{
	// store Z80 registers in this SnaHead
	// does not store PC
	// PC must be pushed on stack BEFORE calling storeRegisters()

	xlogIn("SnaHead:setRegisters");

	iff = regs.iff1 ? 0x04 : 0x00;
	im	= regs.im;

	i	= regs.i;	r	= regs.r;
	a	= regs.a;	a2	= regs.a2;
	f	= regs.f;	f2	= regs.f2;
	b	= regs.b;	c	= regs.c;
	d	= regs.d;	e	= regs.e;
	h	= regs.h;	l	= regs.l;
	xh	= regs.xh;	xl	= regs.xl;
	yh	= regs.yh;	yl	= regs.yl;
	b2	= regs.b2;	c2	= regs.c2;
	d2	= regs.d2;	e2	= regs.e2;
	h2	= regs.h2;	l2	= regs.l2;
	sph = regs.sph;	spl = regs.spl;
}

void SnaHead::getRegisters(Z80Regs& regs)
{
	// set Z80 registers from SnaHead
	// does not set the PC
	// PC must be popped from stack AFTER calling getRegisters()

	xlogIn("SnaHead:getRegisters");

	regs.i 	= i;    regs.r 	= r;
	regs.a 	= a;	regs.f	= f;
	regs.a2	= a2;	regs.f2	= f2;
	regs.b	= b;	regs.c	= c;
	regs.d	= d;	regs.e	= e;
	regs.h	= h;	regs.l	= l;
	regs.xh	= xh;	regs.xl	= xl;
	regs.yh	= yh;	regs.yl	= yl;
	regs.b2	= b2;	regs.c2	= c2;
	regs.d2	= d2;	regs.e2	= e2;
	regs.h2	= h2;	regs.l2	= l2;
	regs.sph= sph;  regs.spl= spl;

	regs.im	  = (im&3)<3 ? im&3 : 1/*rst7*/;
	regs.iff1 = regs.iff2 = (iff&0x04) ? enabled : disabled;

	if(XLOG)
	{
		logline("  a f  b c  d e  h l a2f2 b2c2 d2e2 h2l2  i x  i y   pc   sp  iff  i r   im");
		for(int i=0;i<16;i++) log(" %04x",regs.nn[i]); logNl();
	}
}

void MachineZxsp::saveSna(FD &fd ) throws
{
	// Save snapshot into .sna file
	// note: .sna files were originally saved from an NMI routine.
	// therefore the PC is stored on the Z80 stack and only IFF1 needs to be saved

	xlogIn("Machine:writeSna");
	assert(ram.count()<=48 kB);

	SnaHead head;
	cpu->push2(cpu->getRegisters().pc);
	head.setRegisters(cpu->getRegisters());
	(void)cpu->pop2();
	head.brdr = ula->getBorderColor();
	head.writeToFile(fd);

	uint32 ramsize = ram.count();                   // note: includes external ram, if any
	for( uint32 i=0; i<ramsize; i+=CPU_PAGESIZE )
	{
		write_mem( fd, cpu->rdPtr(0x4000+i), CPU_PAGESIZE );
	}
}

void MachineZxsp::loadSna(FD &fd ) throws
{
	// Load snapshot from .sna file
	// throws on error
	// may adjust model
	// shows warning alert if SP is out of ram
	// --> machine is powered on but suspended

	xlogIn("Machine:readSna");

	assert(ram.count()<=48 kB);
	assert(is_locked());

	uint32 qramsize = fd.file_size()-snalen;  if(qramsize>0xC000) qramsize=0xc000;
	assert(ram.count()/*zramsize*/>=qramsize);

	// we need to power on the machine but it must not runForSound()
	// don't block: we might be called from runForSound()!
	_suspend();
	_power_on();

	SnaHead head;
	head.readFromFile(fd);
	head.getRegisters(cpu->getRegisters());

	if(crtc->isaId() == isa_SpectraVideo) SpectraVideoPtr(crtc)->setVideoMode(0);
	crtc->setBorderColor(head.brdr);

	for( uint32 i=0; i<qramsize; i+=CPU_PAGESIZE )
	{
		xlog("x"); read_mem( fd, cpu->wrPtr(0x4000+i), CPU_PAGESIZE );
	}

	uint16& sp = cpu->getRegisters().sp;
	if(sp>=0x4000+qramsize) showWarning("Stack beyond ram: $%4X", uint(sp));
	cpu->getRegisters().pc = cpu->pop2();
	if(sp<=0x4000) showWarning("Stack within rom: $%4X", uint(sp));
	xlogline(" snapshot loaded ok");
}


// ###############################################################################
//                  ROM save / load Patches
// ###############################################################################

#if 0
/*  save data as with the ZXSP ROM 'save tape' routine
	nominal routine address: 0x04D0
	• addr = IX
	• size = DE
	• pilot length = HL
	• type byte = A
	• return address = (SP++)
	return:	registers set for 'ok'
*/
int TapData::save( Z80& cpu ) noexcept			// zx80 rom save patch
{
	xlogIn("TapData:Save");

	purge();

	Z80Regs& regs = cpu.getRegisters();
	uint16 addr  = regs.ix;	// data address
	uint16 cnt   = regs.de;	// data size
//	uint16 pilot = regs.hl;	// pilot pulses		TODO: warn, if hl is non-standard
	uint8  a	 = regs.a;	// type byte

// preset to 'breaked':
	regs.ix++;				// first 'break' test is after first saved byte
	regs.de--;
	regs.a = 0xFE>>1;		// RRA:	a = (a>>1) + (f<<7)
	regs.f = 0x00;			//		f = (f&~(C_FLAG+N_FLAG+H_FLAG))
	regs.pc = cpu.pop2();	// RET NC

// store data:
	data.grow(int32(cnt+2));
	data[0] = a;
	cpu.copyRamToBuffer(addr,&data[1],cnt);
	data[cnt+1] = calc_zxsp_tapeblock_crc(&data[1],cnt,a);

// return 'ok':
	regs.ix += cnt;			// += regs.de+1
	regs.de = 0xFFFF;		// -= regs.de+1
	regs.a	= 0;			//			after jp nz,BYTEAU	; $04FE
	regs.b  = 0;			//			after DJNZ
	regs.f  |= C_FLAG;		// 'ok'		after jp nz,BYTEAU	; $04FE

	return 0;				// handled
}


/*	load data from buffer to ram file as with the ZX Spectrum Rom load routine
	nominal patch address: 0x0556
	in:
		IX:	addr
		DE:	count		(D must not be $FF)
		A:	blocktype	(byte 1)
		F:	CY = load
			NC = verify
	ok:
		CY,NZ = ok
		A	  = 0 (crc)
		B	  = 0xB0
		C     = 0x02 oder 0x22  je nach letztem ear_in bit
		H     = 0 (crc)
		L     = last (crc) byte
		IX	 += count
		DE	  = 0		( -= count)
		AF,BC,DE,HL,AF',IX modified
		IFF	disabled	(will be enabled in SAVLOA)
		PC	return address popped from stack
		• SAVLOA ($053F) wird anfangs auf den Stack gepusht und kann dort überschrieben werden
		• FLANK2 ($05E3) und darin FLANK1 ($05E7) werden für jedes Bit aufgerufen und überschreiben die geladenen Daten
	err:
		NC    = break or error
		NC NZ = break						IX,DE entspr. geladenen bytes
		NC NZ = wrong block type			IX,DE untouched;  EX AF,AF';  L = block type found
		NC NZ = verify error				IX,DE entspr. wrong byte;  EX AF,AF';
		NC Z  = error (pulse too long)
		NC Z  = final CRC == 1				IX,DE entspr. count;  A = H = 1;  L = last (crc) byte
		NC NZ = final CRC wrong (but not 1)	IX,DE entspr. count;  A = H > 1;  L = last (crc) byte
*/
int TapData::load( Z80& cpu ) const noexcept	// zx80 rom load patch
{
	xlogIn("TapData:Load");

	if(data.count()==0) return -1;	// not handled

	Z80Regs& regs   = cpu.getRegisters();
	uint16   zaddr  = regs.ix;				// target address
	uint32   zlen   = regs.de +2;			// target data size incl. block_type and crc
	uint8    ztype  = regs.a;				// target block type
	bool     verify = !(regs.f & C_FLAG);	// flag: verify data, else load data
	uint32	 qlen   = data.count();			// source data size
	bool	 trunc  = qlen<zlen;			// flag: truncated, source data too short

// preset for 'error':
// • wie nach FLANK2 test failed:
// • IX, DE untouched (no bytes stored/verified)
	cuint16 SAVLOA    = 0x053F;		// return address pushed on stack at start of routine
	cuint16 FLANK2RET = 0x05CD;		// ret addr on stack when calling FLANK2
	cuint16 FLANK1RET = 0x05E6;		// ret addr on stack when calling FLANK1
	cpu.push2(SAVLOA);
	cpu.push2(FLANK2RET);
	cpu.push2(FLANK1RET);
	regs.sp += 6;					// undo pushes
	regs.pc = SAVLOA;				// ~ RET
	regs.iff1 = regs.iff2 = disabled;
	regs.c = 0x22;					// ear_in bit
	regs.f = Z_FLAG;				// NC & Z

// exclude block type and crc from buffers:
	cu8ptr q = data.Data();
	uint8 qtype = *q;				// block type on tape
	q++; qlen--; zlen--;			// skip type byte
	if(!trunc) { qlen--; zlen--; }	// if not truncated then exclude the crc byte from buffer data

// check block type:
	if( qtype != ztype )
	{
		regs.l = qtype;				// block type found
		regs.a = qtype ^ ztype;
		regs.f = 0x00;				// NC,NZ  ->  error
		// TODO: alert box
		return 0;					// handled
	}

// load or verify bytes:
	if( verify )					// verify
	{
		u8ptr z = (u8ptr)tempMem(qlen);
		cpu.copyRamToBuffer(zaddr,z,qlen);
		for( uint32 i=0; i<qlen; i++ )
		{
			if( q[i] != z[i] ) { regs.ix+=i; regs.de-=i; regs.f=0x00; /*verify error*/ return 0; /*handled*/ }
		}
	}
	else							// load
	{
		cpu.copyBufferToRam(q,zaddr,qlen);
		regs.sp -= 2;				// redo PUSH SAVLOA which may be overwritten
		cpu.push2(FLANK2RET);		// redo
		cpu.push2(FLANK1RET);		// redo
		regs.sp += 4;				// undo 2x pushes
		regs.pc = cpu.pop2();		// redo RET to SAVLOA, hopefully
	}

// check load error: block on tape too short
	if( trunc )
	{
		regs.ix += qlen;
		regs.de -= qlen;
		regs.f = Z_FLAG;			// NC,Z: pulse too long
		return 0;					// handled
	}

// ok or crc error:
	uint8 crc = calc_zxsp_tapeblock_crc(q,qlen,qtype);		// calculated crc for qtype + qdata
	regs.h = regs.a = crc ^= regs.l = q[qlen];				// actual crc on tape;  final XOR must result in 0x00
	regs.ix += qlen;
	regs.de  = 0;
	regs.b = 0xB0;
	regs.f = crc==0 ? C_FLAG : crc==1 ? Z_FLAG : 0x00;		// CY,NZ = ok; else crc error

	return 0;						// handled
}
#endif



