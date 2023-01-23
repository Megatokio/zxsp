// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Z80Head.h"
#include "ZxInfo/ZxInfo.h"
#include "unix/FD.h"


/*	Z80 Version 1.45 Header

		uint8 a,f,c,b,l,h,pcl,pch,spl,sph,i,r;

		uint8 data		Bit 0  : Bit 7 of the R-register
						Bit 1-3: Border colour
						Bit 4  : 1=Basic SamRom switched in
						Bit 5  : 1=Block of data is compressed (v1.45 only)
						Bit 6-7: unused
*zxsp*					Bit 5-7: bits 5-7 of the SPECTRA border color

		uint8 e,d,c2,b2,e2,d2,l2,h2,a2,f2;
		uint8 yl,yh,xl,xh,iff1,iff2;

		uint8 im		Bit 0-1: Interrupt mode (0, 1 or 2)
						Bit 2  : 1=Issue 2 emulation
*zxsp*							 b&w models: 1=60Hz
						Bit 3  : 1=Double interrupt frequency
						Bit 4-5: 1=High video synchronisation
								 3=Low video synchronisation
								 0,2=Normal
						Bit 6-7: 0=Cursor/Protek/AGF joystick
								 1=Kempston joystick
								 2=Sinclair 2 Left joystick (or user defined, for version 3 .z80 files)
								 3=Sinclair 2 Right joystick

	Z80 version 2.01 enhanced header if PC=0:

		uint16 h2len	size of header extension: 23 for version 2.01 files
		uint16 npc		pc (instead of pcl+pch, which is set to 0)

		uint8 model		Value	Meaning in v2.01	Meaning in v3.0		ldiremu.bit7	im.bit2
						0		48k					48k					16k				issue2
						1		48k + If.1			48k + If.1			16k				issue2
						2		SamRam				SamRam				16k				issue2
						3		128k				48k + M.G.T.		16k				issue2
						4		128k + If.1			128k				+2
						5		-					128k + If.1			+2
						6		-					128k + M.G.T.		+2
						7,8		-					+3					+2A
						9		-					Pentagon 128k
						10		-					Scorpion 256k
						11		-					Didaktik-Kompakt
						12		-					+2
						13		-					+2A
						14		-					TC2048
						15		-					TC2068

*zxsp*					76		-					TK85
*zxsp*					77      -                   TS1000								60 Hz
*zxsp*					78		-					TS1500								60 Hz
*zxsp*					80		-					ZX80								60 Hz
*zxsp*					81		-					ZX81								60 Hz
*zxsp*					83		-					Jupiter ACE							60 Hz
*zxsp*					84		-					Inves 48k
*zxsp*					85		-					+128 Span.
*zxsp*					86		-					Sam Coupé
*zxsp*					87		-					+2 Spanish
*zxsp*					88		-					+2 French
*zxsp*					89		-					+3 Spanish
*zxsp*					90		-					+2A Spanish
*zxsp*					91		-					tk90x
*zxsp*					92		-					tk95
						128		-					TS2068

		uint8 port_7ffd or port_f4:
								If in SamRam mode, bitwise state of 74ls259.
								For example, bit 6=1 after an OUT 31,13 (=2*6+1)
								If in 128 mode, contains last OUT to 7ffd (paging control)
								if timex ts2068: last out to port 244
		uint8 if1paged or port_ff:
								!=0 means: interface1 rom is paged in
								if timex ts2068: last out to port 255
		uint8 rldiremu			Bit 0: 1 if R register emulation on
								Bit 1: 1 if LDIR emulation on
								Bit 2: AY sound in use, even on 48K machines
*zxsp*							Bit 3: SPECTRA interface present, can only add to zxsp 16/48k, +128k and +2/+3A
*zxsp*							Bit 5: if zxsp, then present a ZXSP Plus
								Bit 6: (if bit 2 set) Fuller Audio Box emulation
								Bit 7: Modify hardware (see below)
		uint8 port_fffd			Last OUT to fffd (soundchip register number)
		uint8 soundreg[16]		Contents of the sound chip registers

	Z80 version 3.0 enhanced header if PC=0 & h2len≥54:

		uint8 t_l,t_m,t_h		T state counter
		uint8 spectator			Flag byte used by Spectator (QL spec. emulator)
								Ignored by Z80 when loading, zero when saving
*zxsp*							ram size [kB] for model 76 - 83
		uint8 mgt_paged			0xFF if MGT Rom paged
		uint8 multiface_paged	0xFF if Multiface Rom paged. Should always be 0.
		uint8 ram0,ram1			0xFF if 0-8191 / 8192-16383 is ROM
		uint8 joy[10]			5* ascii word: keys for user defined joystick
		uint8 stick[10]			5* keyboard mappings corresponding to keys above
		uint8 mgt_type			MGT type: 0=Disciple+Epson,1=Disciple+HP,16=Plus D
		uint8 disciple_inhibit_button_status	0=out, 0xFF=in
		uint8 disciple_inhibit_flag				Disciple inhibit flag: 0=rom pageable, 0xFF=not

	warajewo/xzx extension if PC=0 & h2len≥55:

		uint8 port_1ffd			last out to $1ffd ((bank switching on +3))
*zxsp*	uint8 spectra_bits		if SPECTRA present:
*zxsp*							Bit 0: new colour modes enabled
*zxsp*							Bit 1: RS232 enabled
*zxsp*							Bit 2: Joystick enabled
*zxsp*							Bit 3: IF1 rom hooks enabled
*zxsp*							Bit 4: rom paged in
*zxsp*							Bit 5: port 239: Comms out bit
*zxsp*							Bit 6: port 239: CTS out bit
*zxsp*							Bit 7: port 247: Data out bit
*zxsp*	uint8 spectra_port_7fdf	if SPECTRA present: last out to port 7FDF (colour mode register)
*/




/*  read a .z80 header
	reads v1.45, ≥v2.01 header of any size (( ≤ sizeof(Z80Head) ))
*/
void Z80Head::read(FD& fd) throws
{
	clear();
	fd.read_bytes(this,z80v1len);
	if(data==0x0ff) data = 1;
	if(isVersion145()) return;

	fd.read_bytes(&h2lenl,2);               // h2lenl, h2lenh

	uint16 xtlen  = 256*h2lenh + h2lenl;
	uint16 maxlen = z80maxlen-z80v1len-2;

	if(xtlen<=maxlen)
	{
		fd.read_bytes(((ptr)this)+z80v1len+2,xtlen);   // v2.01 or v3.0 or later
	}
	else
	{
		fd.read_bytes(((ptr)this)+z80v1len+2,maxlen);  // extended v3.0
		fd.seek_fpos(z80v1len+2+xtlen);
	}
}


/*  write a .z80 header
	writes header data as indicated by h2lenh*256+h2lenl
	if h2lenh==h2lenl==0 then writes header without trailing zeros
*/
void Z80Head::write(FD& fd) throws
{
	assert(h2lenh==0);
	assert(h2lenl == z80v2len-2-z80v1len || h2lenl >= z80v3len-2-z80v1len);	// other emulators are soo picky…

//    uint sz = h2lenl;               // size of header extension
//    if(sz==0)                       // header size not set?
//    {
//        ptr p = ((ptr)this) + z80maxlen;
//        while(p[-1]==0) { p--; }
//        sz = p-((ptr)this) - (z80v1len+2);
//        if(sz<26) sz=26;			// include T state counters
//        h2lenl = sz;
//    }

	fd.write_bytes(this,z80v1len+2+h2lenl);
}


/*  set model from zxsp model number
	TODO: currently the zxplus_span is saved as inves
*/
void Z80Head::setZxspModel(Model zxmodel, bool if1, bool mgt)
{
	switch(zxmodel)
	{
	case zxsp_i1:		rldiremu |= 0x80;			// 16K
	case zxsp_i2:       im |= 4;                    // issue2
	case zxsp_i3:		if(0)
	case zxplus:		rldiremu |= 0x20;			// plus
						model = if1 ? 1 : mgt ? 3 : 0;
						break;

	case zxplus2:       rldiremu |= 0x80;            // +2
	case zx128:         model = if1 ? 5 : mgt ? 6 : 4; break;

	case tk85:			model = 76; break;	// 60 Hz
	case ts1000:		model = 77; break;	// 60 Hz
	case ts1500:		model = 78; break;	// 60 Hz
	case zx80:          model = 80;	break;
	case zx81:          model = 81; break;
	case jupiter:       model = 83;	break;
	case zxplus_span:
	case inves:         model = 84; break;

	case zx128_span:    model = 85; break;

	case zxplus2_span:  model = 87; break;
	case zxplus2_frz:	model = 88; break;

	case zxplus3:       model = 7;  break;
	case zxplus3_span:  model = 89; break;

	case zxplus2a:      model = 7; rldiremu |= 0x80; break;
	case zxplus2a_span: model = 90; break;

	case tc2048:        model = 14; break;
	case tc2068:        model = 15; break;
	case ts2068:        model = 128; break;
	case pentagon128:	model = 9;  break;
	case scorpion:      model = 10; break;
	case samcoupe:      model = 86; break;

	case tk90x:			model = 91; break;
	case tk95:			model = 92; break;

	default: TODO();
	}
}

/*  get zxsp-style model id from header data
	returns -1 on error or not supported
*/
Model Z80Head::getZxspModel()
{
	if(isVersion145())
	{
		return im&4 ? zxsp_i2 : zxsp_i3;
	}

	uint8 model = this->model;

	if(isVersion201())
	{
		//	model:        Meaning in v2           Meaning in v3
		//	-----------------------------------------------------
		//	0             48k                     48k
		//	1             48k + If.1              48k + If.1
		//	2             SamRam                  SamRam
		//	3             128k                    48k + M.G.T.
		//	4             128k + If.1             128k
		//	5             -                       128k + If.1
		//	6             -                       128k + M.G.T.
		if(model==3||model==4) model += 1;
	}

// Version 3.0 and later:

		//	model:          Meaning
		//	-----------------------------------------------------
		//	  7             Spectrum +3
		//	  8             [mistakenly used by some versions of XZX-Pro to indicate a +3]
		//	  9             Pentagon (128K)
		//    10            Scorpion (256K)
		//    11            Didaktik-Kompakt
		//    12            Spectrum +2
		//    13            Spectrum +2A
		//    14            TC2048
		//    15            TC2068
		//   128            TS2068

		// While most emulators using these extensions write version 3 files, some write version 2 files
		// so it's probably best to assume any of these values can be seen in either version 2 or version 3 files.

		//  rldiremu    Bit 7 = 1: Modify hardware
		//				If bit 7 is set, the hardware types are modified slightly:
		//				any 48K machine becomes a 16K machine,
		//				any 128K machines becomes a +2
		//				and any +3 machine becomes a +2A.
		//	im          Bit 2 = 1: Issue2 emulation

	switch(model)
	{
	case 2:     //return unknown_model;	// SamRam
	case 0:
	case 1:
	case 3:     return rldiremu>>7 ? zxsp_i1 : im&4 ? zxsp_i2 : (rldiremu>>5)&1 ? zxplus : zxsp_i3;    // 46k, modify hw: 16k

	case 4:
	case 5:
	case 6:     return rldiremu>>7 ? zxplus2 : zx128;                       // 128, modify hw: +2

	case 7:
	case 8:     return rldiremu>>7 ? zxplus2a : zxplus3;                    // +3, modify hw: +2A

	case 9:     return pentagon128;		// Pentagon 128k
	case 10:    return scorpion;        // Scorpion 256k
	case 11:    return unknown_model;	// Didaktik-Kompakt
	case 12:    return zxplus2;
	case 13:    return zxplus2a;
	case 14:    return tc2048;
	case 15:    return tc2068;

	case 76:	return tk85;			// TK85			(kio)
	case 77:    return ts1000;			// TS1000		(kio)
	case 78:    return ts1500;			// TS1500		(kio)
	case 80:    return zx80;            // ZX80			(kio)
	case 81:    return zx81;            // ZX81			(kio)
	case 83:    return jupiter;         // Jupiter ACE  (kio)
	case 84:    return inves;           // Inves 48k	(kio)
	case 85:    return zx128_span;      // +128 Span.	(kio)
	case 86:    return samcoupe;        // Sam Coupé	(kio)
	case 87:    return zxplus2_span;    // +2 Spanish	(kio)
	case 88:    return zxplus2_frz;		// +2 French	(kio)
	case 89:    return zxplus3_span;    // +3 Spanish	(kio)
	case 90:    return zxplus2a_span;   // +2A Spanish	(kio)
	case 91:	return tk90x;			// TK90X		(kio)
	case 92:	return tk95;			// TK95			(kio)
	case 128:   return ts2068;

	default:	return unknown_model;	// error
	}
}


/*	get required ramsize for machine
	returns 0 for default ram size
*/
uint32 Z80Head::getRamsize()
{
	uint model = this->model;									// 2014-10-23
	if(isVersion201() && (model==3||model==4)) model += 1;		// 2014-10-23

	if(model<=3) return rldiremu>>7 ? 16 kB : 48 kB;
	if(varyingRamsize()) return spectator * 1 kB;
	return 0;
}


void Z80Head::setCpuCycle(int32 cc, int cc_per_frame)
{
// T state counter: (byte 55-57)
// The hi T state counter counts up modulo 4. Just after the ULA generates its once-in-every-20-ms interrupt,
// it is 3, and is increased by one every 5 emulated milliseconds.
// In these 1/200s intervals, the low T state counter counts down from 17471 to 0 (17726 in 128K modes),
// which make a total of 69888 (70908) T states per frame.
	int32 cc_per_5ms = cc_per_frame / 4;
	t_h = ((3 + cc/cc_per_5ms) & 3);	cc = cc % cc_per_5ms;  cc = cc_per_5ms -1 - cc;
	t_m = cc/256;
	t_l = cc%256;
}


int32 Z80Head::getCpuCycle(int cc_per_frame)
{
	int32 cc_per_5ms = cc_per_frame / 4;
	int32 n = ((t_h-3)&3) * cc_per_5ms + cc_per_5ms -1 - uint16(t_m*256+t_l);
	limit( 0, n, cc_per_frame );
	return n;
}


/*  store registers into Z80Head
	clears to all-zero and then sets the registers
	some bytes used unused bits for some flags
	these can be added after this call by ORing
*/
void Z80Head::setRegisters(Z80Regs const& reg)
{
	clear();

	a	= reg.a;		a2	= reg.a2;
	f	= reg.f;		f2	= reg.f2;
	b	= reg.b;		c	= reg.c;
	d	= reg.d;		e	= reg.e;
	h	= reg.h;		l	= reg.l;
	xh	= reg.xh;		xl	= reg.xl;
	yh	= reg.yh;		yl	= reg.yl;
	b2	= reg.b2;		c2	= reg.c2;
	d2	= reg.d2;		e2	= reg.e2;
	h2	= reg.h2;		l2	= reg.l2;

	sph  = reg.sph;     spl  = reg.spl;
	npch = reg.pch;     npcl = reg.pcl;

	iff1 = reg.iff1;    iff2 = reg.iff2;
	i    = reg.i;       r	 = reg.r & 0x7F;
	data = reg.r>>7;	im	 = reg.im;
}


void Z80Head::getRegisters(Z80Regs& reg) const
{
	reg.a 	= a;		reg.f	= f;
	reg.a2	= a2;		reg.f2	= f2;
	reg.b	= b;		reg.c	= c;
	reg.d	= d;		reg.e	= e;
	reg.h	= h;		reg.l	= l;
	reg.xh	= xh;		reg.xl	= xl;
	reg.yh	= yh;		reg.yl	= yl;
	reg.b2	= b2;		reg.c2	= c2;
	reg.d2	= d2;		reg.e2	= e2;
	reg.h2	= h2;		reg.l2	= l2;

	if(isVersion145()){ reg.pch	= pch;		reg.pcl	= pcl; }
	else              { reg.pch	= npch;		reg.pcl	= npcl; }
	reg.sph	= sph;		reg.spl	= spl;

	reg.iff1= iff1;  	reg.iff2= iff2;
	reg.i 	= i;    	reg.r 	= (r&0x7f) + (data<<7);
	reg.im	= im&3; if (reg.im>2) reg.im = 1;
}


/*  determine required model for loading this .z80 snapshot.
	Return unknown_model = Error or not supported.
*/
Model modelForZ80(FD& fd) throws
{
	Z80Head head;
	off_t p = fd.file_position();
	head.read(fd);
	fd.seek_fpos(p);

	return head.getZxspModel();
}



















