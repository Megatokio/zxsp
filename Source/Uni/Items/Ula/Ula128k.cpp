/*	Copyright  (c)	Günter Woigk 2012 - 2019
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


#include "Ula128k.h"
#include "Z80/Z80options.h"
#include "Z80/Z80.h"
#include "Machine/Machine.h"


// o_addr_128	=	"0---.----.----.--0-"		// üblicher Port: 0x7ffd
// o_addr_48k	=	"----.----.----.---0"		// übliche Adresse: $FE     BESTÄTIGT
#define o_addr		"----.----.----.----"
#define i_addr		"----.----.----.---0"


Ula128k::Ula128k(Machine*m)
:
	UlaZxsp(m,isa_Ula128k,o_addr,i_addr)
{}


Ula128k::Ula128k(Machine*m, isa_id id, cstr oaddr, cstr iaddr)
:
	UlaZxsp(m,id,oaddr,iaddr),
	port_7ffd(0)
{}


void Ula128k::powerOn(/*t=0*/ int32 cc)
{
	port_7ffd = 0;
	UlaZxsp::powerOn(cc);
	//markVideoRam();
}


void Ula128k::reset( Time t, int32 cc )
{
	port_7ffd = 0;
	UlaZxsp::reset(t,cc);
	markVideoRam();
}



void Ula128k::output( Time t, int32 cc, uint16 addr, uint8 byte )
{
	if(~addr&1)
	{
		UlaZxsp::output(t,cc,addr,byte);
	}
	else
	{
		// test for video page change:
		// o_addr_128 = "0---.----.----.--0-"	// üblicher Port: 0x7ffd

		if(addr & 0x8002) return;				// not the MMU port
		if(mmu_is_locked()) return;				// mmu port disabled
		if((byte^port_7ffd) & 0x08)				// video page changed?
		{
			if(cc >= ccx) updateScreenUpToCycle(cc);
			port_7ffd = byte;
			markVideoRam();
		}
	}
}


/*	callback from Mmu128k::setPort7FFD()
	update video ram page
*/
void Ula128k::setPort7ffd( uint8 byte )
{
	port_7ffd = byte;
	markVideoRam();
}


void Ula128k::markVideoRam()
{
	#define SET(A,SZ)	if(~*A & cpu_crtc) for( CoreByte *j=A, *e=A+SZ; j<e; j++ ) *j |= cpu_crtc;
	#define RES(A,SZ)	if( *A & cpu_crtc) for( CoreByte *j=A, *e=A+SZ; j<e; j++ ) *j &= uint32(~cpu_crtc);

	uint page = port_7ffd&0x08 ? 7 : 5;

	CoreByte* v = video_ram = &ram[page*0x4000];
	if(screen) { SET(v,6912); } else { RES(v,6912); }
	//if((v[0] & cpu_crtc)==0) for( uint32 j=0, e=6912; j<e; j++ ) v[j] |= cpu_crtc;

	CoreByte* w = &ram[(5+7-page)*0x4000];
	RES(w,6912);
	//if((w[0] & cpu_crtc)!=0) for( uint32 j=0, e=6912; j<e; j++ ) w[j] &= ~cpu_crtc;
}


int32 Ula128k::addWaitCycles( int32 cc, uint16 addr ) volatile const
{
	if(cc<cc_waitmap_start || cc>=cc_waitmap_end)  return cc;	// not in screen

	/*	waitstates for any i/o:
		• waitstates are added for contended memory access
		• waitstates are added for ula access (i/o with A0=0)
		• waitstates are added for non-ula i/o with address matching contended memory page
		  • not for +3
		  • on +128, this applies to $4000 and $C000 if contended memory is currently paged in
		• waitstates are inserted as for memory, if the i/o is "contended"

		•	A14,A15 =   |       |
			cont. page? | bit 0 | Contention pattern
			------------+-------+-------------------
				 No     |   0   | N:1, C:3
				Yes     |   0   | C:1, C:3
				 No     |   1   | N:4
				Yes     |   1   | C:1, C:1, C:1, C:1
	*/

	// pages 0 and 2 never contended
	// page 1 always contended
	// page 3 only if screen paged in
	bool contended = (addr&0x4000) && ( (~addr&0x8000) || (port_7ffd&5)==5 );

	if(addr & 0x0001)	// ULA not accessed:
	{
		if(contended)
		{
			// access to address which looks like a screen memory access:
			cc += waitmap[(cc-1)%cc_per_line];		// -1 .. +2   --> vgl. BorderBarGenerator
			cc += waitmap[(cc+0)%cc_per_line];
			cc += waitmap[(cc+1)%cc_per_line];
			cc += waitmap[(cc+2)%cc_per_line];
			cpu->setCpuCycle(cc);
		}
	}
	else	// ULA accessed:
	{
		if(contended) cc += waitmap[(cc-1)%cc_per_line];
		cc += waitmap[(cc+0)%cc_per_line];
		cpu->setCpuCycle(cc);
	}
	return cc;
}
















