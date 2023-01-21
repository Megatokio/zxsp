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

#include "MachineJupiter.h"
#include "TapeRecorder.h"
#include "Audio/TapData.h"
#include "Settings.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/UlaJupiter.h"
#include "Ula/MmuJupiter.h"


// bit masks for z80 flag register:
	#define	S_FLAG	0x80
	#define	Z_FLAG	0x40
	#define	H_FLAG	0x10
	#define	P_FLAG	0x04
	#define	V_FLAG	0x04
	#define	N_FLAG	0x02
	#define	C_FLAG	0x01


MachineJupiter::MachineJupiter(MachineController* m)
:
	Machine(m,jupiter,isa_MachineJupiter)
{
	cpu			= new Z80(this);
	ula			= new UlaJupiter(this,settings.get_bool(key_framerate_jupiter_60hz,false)?60:50);
	mmu			= new MmuJupiter(this);
	keyboard	= new KeyboardJupiter(this);
	//ay		=
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);
}

static uint8 calc_zxsp_tapeblock_crc ( uint8 const* data, int cnt ) noexcept
{
	uint8 crc = 0;
	for( int i=0; i<cnt; i++ ) { crc ^= data[i]; }
	return crc;
}

bool MachineJupiter::handleSaveTapePatch()
{
	// handle rom patch
	// called with all z80 registers stored
	// return 0: patch not handled => Z80 resumes with current instruction
	// return 1: full cc and irpt test & fetch next instr(PC++)

	xlogIn("MachineJupiter:handleSaveTapePatch");

// test whether the tape recorder can record a block:
	if(!taperecorder->can_store_block()) return 0;		 // not handled

// get registers:
	Z80Regs& regs = cpu->getRegisters();
	if(regs.pc != 0x1829) return 0;		// patch address = short after TXALL = $1820

	uint dataaddr = regs.iy;		// IY = data address
	uint datalen  = regs.de;		// DE = count
	uint flagbyte = regs.c;			// C = flag byte
									// address of TXRXQUIT on stack

	uint32 ppilot = flagbyte&0x80 ? 0x0400 : 0x2000;	// num. pilot pulses
	Time   pause  = flagbyte&0x80 ? 1.000  : 0.002;		// pause after block

// store data
	Array<uint8> buffer(datalen+2);
	buffer[0] = flagbyte;
	cpu->copyRamToBuffer(dataaddr,&buffer[1],datalen);
	buffer.last() = calc_zxsp_tapeblock_crc(&buffer[1],datalen);
	taperecorder->storeBlock(new TapData(buffer, ppilot, pause, no/*jupiterAce*/));

// set registers:
	regs.pc = cpu->pop2();			// TXRXQUIT
	regs.de = -1;
	regs.hl = 0;
	regs.bc = 0x2e08;
	regs.a  = 0xff;
	regs.f  = Z_FLAG | N_FLAG;
	regs.iy = dataaddr + datalen + 1;

	return 1;   // handled
}

bool MachineJupiter::handleLoadTapePatch()
{
	xlogIn("MachineJupiter::handleLoadTapePatch");

// test whether the tape recorder can read a block:
	if(!taperecorder->can_read_block()) return 0;	// not handled - no file loaded etc.

// load the block:
	TapData* bu = taperecorder->getZxspBlock();
	if(!bu) return 0;								// not handled - no block found

	//  load data from tape file as with the ZX Spectrum Rom load routine
	//  nominal patch address: 0x18B1 (zxsp=0x0556)
	//  behaves very similar to Specci routine.
	//
	//  in	IX = address
	//  	DE = count				(D ≠ 0xFF)
	//  	C  = block type: 0x00 = header, 0xFF = data
	//  	CY = load; NC = verify
	//
	//  do	IX = current address
	//  	DE = remaining count
	//  	L  = current (last read) byte
	//  	H  = current crc: starts with requested blocktype, next xored with actual blocktype,
	//  					  so seemingly starting with 0x00 after blocktype
	//
	//  out	IY ~ last byte position		ok => IY += DE
	//  	DE ~ bytes read,			ok => DE = 0
	//  	HL = last values (crc and last byte read)
	//  	A  = CRC,					ok => A = 0
	//  	C  =						ok => 0xFF
	//  	F  =						ok => CY set; F = SHNC
	//  		 break			NC NZ S			C = 0
	//  		 wrong block	NC NZ			ex af,af', iy,de untouched, a=h=0xff, l=actual block type
	//  		 load error		NC Z
	//  		 verify error	NC NZ			ex af,af'
	//  		 crc error		NC				a=h≠0

	cu8ptr q = bu->getData();
	uint32 qlen = bu->count();

	#define RXBYTE_RET	0x18F3		// return address for call to RXBYTE during data[] load/verify
	#define	RXBIT_RET	0x1903		// return address for call to RXBIT from RXBYTE
	#define RX_RET		0x18FB		// plain RET opcode

	Z80Regs& regs   = cpu->getRegisters();
	uint16   zaddr  = regs.iy;				// target address
	uint32   zlen   = regs.de +2;			// target data size incl. block_type and crc
	uint8    ztype  = regs.c;				// target block type
	bool     verify = !(regs.f & C_FLAG);	// flag: verify data, else load data
	bool	 trunc  = qlen<zlen;            // flag: truncated, source data too short
	qlen = min(qlen,zlen);					// 2013-09-14

// preset for 'load error' if we throws:
	regs.f = Z_FLAG;				// NC & Z
	regs.a = regs.h = 0;			// crc after typebyte
	regs.pc = RX_RET;

// exclude block type and crc from buffers:
	uint8 qtype = *q;				// block type on tape
	q++; qlen--; zlen--;			// skip type byte
	if(!trunc) { qlen--; zlen--; }	// if not truncated then exclude the crc byte from buffer data

// check block type:
	if( qtype != ztype )
	{
		// wrong block:	NC NZ
		//	ex af,af'				(ignored)
		//	iy,de untouched
		//	a = h = qtype^ztype		(most times: 0xFF = 0x00^0xFF)
		//	l = actual block type

		regs.l = qtype;
		regs.a = regs.h = qtype ^ ztype;
		regs.f = 0x00;
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
				// verify error: NC NZ
				// ex af,af'			(ignored)
				// iy,de progressed
				// h = current crc
				// l = byte from tape
				// a = byte from ram ^ byte from tape

				regs.f   = 0x00;
				regs.iy += i;
				regs.de -= i;
				regs.h   = calc_zxsp_tapeblock_crc(q,i);
				regs.l   = q[i];
				regs.a   = q[i] ^ z[i];
				return 1;			// handled - verify error
			}
		}
	}
	else							// load
	{
		cpu->copyBufferToRam(q,zaddr,qlen);
		cpu->push2(RXBYTE_RET);		// redo
		cpu->push2(RXBIT_RET);		// redo
		regs.sp += 4;				// undo 2x pushes
	}

// check load error: block on tape too short
	if( trunc )
	{
		// load error: NC Z
		// l = last byte
		// h = last crc

	//	regs.f   = Z_FLAG;			// NC,Z: pulse too long
		regs.iy += qlen;
		regs.de -= qlen;
		regs.l   = q[qlen-1];
		regs.h   = calc_zxsp_tapeblock_crc(q,qlen);
		return 1;					// handled
	}

// ok or crc error:
//		IY += DE
//		DE = 0
// crc error: NC, a=h≠0
// ok:	CY set; F = SHNC
//		l = last byte
//		a = h = last crc = 0
//		C = 0xFF

	regs.iy += qlen;
	regs.de  = 0;
	regs.a   = regs.h = calc_zxsp_tapeblock_crc(q,qlen+1);
	regs.f   = regs.a==0 ? C_FLAG|S_FLAG|H_FLAG|N_FLAG : 0x00;
	regs.l   = q[qlen-1];
	regs.c   = 0xff;

	return 1;						// handled
}

static void write_compressed_data( FD& fd, uint8 const* q, uint qsize ) throws
{
	// write compressed data
	// compression scheme:
	//		dc.b $ed, count, char

	xlogIn("write_compressed_data");

	assert( qsize>=0x400 && qsize<=0x10000 && (qsize&0x3ff)==0 );

	uint8 const* qe = q + qsize;
	uint8  bu[qsize*4/2];					// worst case size
	uint8* z = bu;

	while( q<qe )
	{
		uint8 c = *q++;

		uint n=1; while(q<qe && *q==c && n<240) { q++; n++; }
		if(c==0xed || n>3)
		{
			*z++ = 0xed;
			*z++ = n;
			*z++ = c;
		}
		else
		{
			while(n--) *z++ = c;
		}
	}

	fd.write_bytes(bu,z-bu);
}

static uint read_compressed_data( FD& fd, uint qsize, uint8* z ) throws
{
	// read compressed data
	// supplied destination buffer z[] must be ≥ $1C000 bytes
	// returns actual size of decompressed data
	// throws on any error

	if(qsize>0x1C000) qsize = 0x1C000;	// worst case qsize
	uint8  bu[qsize]; fd.read_bytes(bu,qsize);
	uint8* q = bu;
	uint8* q_end = q+qsize;
	uint8* z_end = z+0xE000;			// 64K - 8K rom
	uint8* z0 = z;

	while( z<z_end && q<q_end )
	{
		uint c = *q++;					// read next byte

		if(c!=0xed)						// single byte:
		{
			*z++ = c;
		}
		else							// compressed bytes:
		{
			if(q==q_end)  break;		// end of file
			uint n = *q++;				// read next byte: count
			if(n==0)	  return z-z0;	// ok: data end
			if(z+n>z_end) break;		// uncompressed size overflow
			if(q==q_end)  break;		// end of file
			c = *q++;					// read next byte: char
			memset(z,c,n); z += n;		// store bytes
		};
	}

	if(q==q_end) throw FileError(fd,endoffile);
	else throw DataError("decompressed data exceeds maximum size ($E000)");
}

void MachineJupiter::loadAce(FD& fd) noexcept(false) /*file_error,data_error*/
{
	// load snapshot
	// --> machine is powered up but suspended

	xlogIn("MachineJupiter:loadAce(fd)");

	assert(is_locked());

	uint8 bu[0x1C000];
	uint zsize = read_compressed_data(fd,fd.file_size(),bu);

	if(zsize<0x2000) throw DataError("decompressed ram image too short (<$2000)");
	zsize = min(zsize, uint(peek2Z(bu+0x80))-0x2000u);
	if(zsize<0x2000) throw DataError("RAMTOP too low (<$4000)");

	if(zsize==0x2000)	   	//  8k => jupiter 3k without ram extension
	{
		delete findIsaItem(isa_ExternalRam);
	}
	else if(zsize<=0x6000)	// 24k => jupiter 3k with 16k ram extension
	{
		if(ram.count()<19*1024) addExternalItem(isa_Jupiter16kRam);
	}
	else throw DataError("this snapshot needs more than 16K external ram (TODO)");

	if(bu[0x130]>2) throw DataError("invalid interrupt mode (im=%i)",int(bu[0x130]));

	// we need to power on the machine but it must not runForSound()
	// don't block: we might be called from runForSound()!
	_suspend();
	_power_on();

	Z80Regs& regs = cpu->getRegisters();
	regs.af  = peek2Z(bu+0x100);
	regs.bc  = peek2Z(bu+0x104);
	regs.de  = peek2Z(bu+0x108);
	regs.hl  = peek2Z(bu+0x10C);
	regs.ix  = peek2Z(bu+0x110);
	regs.iy  = peek2Z(bu+0x114);
	regs.sp  = peek2Z(bu+0x118);
	regs.pc  = peek2Z(bu+0x11C);
	regs.af2 = peek2Z(bu+0x120);
	regs.bc2 = peek2Z(bu+0x124);
	regs.de2 = peek2Z(bu+0x128);
	regs.hl2 = peek2Z(bu+0x12C);
	regs.im   = bu[0x130];
	regs.iff1 = bu[0x134]&1;
	regs.iff2 = bu[0x138]&1;
	regs.i    = bu[0x13C];
	regs.r    = bu[0x140];

	if(XLOG)
	{
		logline("  a f  b c  d e  h l a2f2 b2c2 d2e2 h2l2  i x  i y   pc   sp  iff  i r   im");
		for(int i=0;i<16;i++) log(" %04x",regs.nn[i]); logNl();
	}

	cpu->copyBufferToRam(bu+0x0400,0x2400,0x400);			// $2400 - $2800: video ram
	cpu->copyBufferToRam(bu+0x0C00,0x2C00,0x400);			// $2C00 - $3000: character ram
	cpu->copyBufferToRam(bu+0x1C00,0x3C00,zsize-0x1C00);	// $3C00 - $4000: built-in program ram
															// $4000++		  external ram
}

void MachineJupiter::saveAce(FD& fd) throws
{
	uint8 bu[0x4000];
	memset(bu,0,0x4000);

	// ACE32 Configuration
	// Addr:   Defaults			Description
	// 2000    01, 80, 00, 00	?
	// 2080    00, 80, 00, 00	Ramtop 4000 (3K), 8000(19K), C000(35K)
	// 2084    00, 00, 00, 00	Debugger Data Address
	// 2088    00, 00, 00, 00	Debugger Breakpoint Address
	// 208C    03, 00, 00, 00	Frame Skip Rate (3)
	// 2090    03, 00, 00, 00	Frames per TV Tick (3)
	// 2094    FD, FD, 00, 00	?
	// 2098    XX, XX, XX, XX	Time emulator is running probably in milliseconds
	// 209C    00, 00, 00, 00	Emulator Colours 0(white on Black), 1(green on Black),
	//											 2(purple on Black), 3(Black on White)

	poke2Z(bu+0x00,0x8001);					// ?
	poke2Z(bu+0x80,13*1024+ram.count());	// RAMTOP
	poke1 (bu+0x8C,3);						// Frame Skip Rate (dummy)
	poke1 (bu+0x90,3);						// Frames per TV tick (dummy)
	poke2 (bu+0x94,0xFDFD);					// ?

	// Z80 Register dump.
	// Addr:	last state		Registers
	// 2100	50, 04, 00, 00		AF
	//		00, 00, 00, 00		BC
	//		E2, 26, 00, 00		DE
	//		28, 3C, 00, 00		HL
	//		00, 3C, 00, 00		IX
	//		C8, 04, 00, 00		IY
	//		FE, 7F, 00, 00		SP
	//		9D, 05, 00, 00		PC
	//		40, 20, 00, 00		AF'
	//		00, 01, 00, 00		BC'
	//		60, 00, 00, 00		DE'
	//		80, 26, 00, 00		HL'
	//		01, 00, 00, 00		IM
	//		01, 00, 00, 00		IFF1
	//		01, 00, 00, 00		IFF2
	//		00, 00, 00, 00		I
	//		11, 00, 00, 00		R
	//		80, 00, 00, 00		?

	Z80Regs& regs = cpu->getRegisters();
	poke2Z(bu+0x100,regs.af);
	poke2Z(bu+0x104,regs.bc);
	poke2Z(bu+0x108,regs.de);
	poke2Z(bu+0x10C,regs.hl);
	poke2Z(bu+0x110,regs.ix);
	poke2Z(bu+0x114,regs.iy);
	poke2Z(bu+0x118,regs.sp);
	poke2Z(bu+0x11C,regs.pc);
	poke2Z(bu+0x120,regs.af2);
	poke2Z(bu+0x124,regs.bc2);
	poke2Z(bu+0x128,regs.de2);
	poke2Z(bu+0x12C,regs.hl2);
	poke1 (bu+0x130,regs.im);
	poke1 (bu+0x134,regs.iff1);
	poke1 (bu+0x138,regs.iff2);
	poke1 (bu+0x13C,regs.i);
	poke1 (bu+0x140,regs.r);
	poke1 (bu+0x144,0x80);

	// $2000 - $2400: all-zero: echo of video ram, fast access for CPU	(ACE32 settings and Z80 registers)
	// $2800 - $2C00: all-zero: echo of character ram, fast access for CPU
	// $3000 - $3C00: all-zero: 3 echoes of built-in program ram

	cpu->copyRamToBuffer(0x2400,bu+0x0400,0x400);	// $2400 - $2800: video ram
	cpu->copyRamToBuffer(0x2C00,bu+0x0C00,0x400);	// $2C00 - $3000: character ram
	cpu->copyRamToBuffer(0x3C00,bu+0x1C00,0x400);	// $3C00 - $4000: built-in program ram
	write_compressed_data(fd,bu,0x2000);

	for(uint i=0;i<ram.count();i+=0x4000)			// $4000++		  ram expansions
	{
		cpu->copyRamToBuffer(i,bu,0x4000);
		write_compressed_data(fd,bu,0x4000);
	}

	static uint8 ed00[2]={0xed,0};
	fd.write_bytes(ed00,2);
}


































