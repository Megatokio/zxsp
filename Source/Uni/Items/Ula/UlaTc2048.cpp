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

#include "UlaTc2048.h"
#include "Templates/Array.h"
#include "Z80/Z80.h"
#include "Qt/Screen/Screen.h"
#include "Machine.h"
#include "MmuTc2048.h"


/*
// WoS FAQ:

	Peripheral: Timex TS2068 ULA.
	Port: ---- ---- 1111 1110 (Same as 48K ULA).

	Peripheral: Timex TS2068 Display mode.
	Port: ---- ---- 1111 1111


TS2068:
	Mode 1: 256x192 Pixels, 24 rows with 32 characters - Uses D_FILE_1 (Hex: 4000-57FF, Dec: 16384-22527) and A_FILE_1 (Hex: 5800-5AFF, Dec: 22528-23296)

	Mode 2: 512x192 Pixels, 24 rows with 64 characters. The INK colour is determined based on the PAPER colour selected. BRIGHT and FLASH are not supported.
		The hi-res screen uses the data area of screen 0 and screen 1 to create a 512x192 pixel screen.
		Columns are taken alternately from screen 0 and screen 1. The attribute area is not used.
		In this mode all colours, including the BORDER, are BRIGHT, and the BORDER colour is the same as the PAPER colour.

	Mode 3: Operationally the same as Mode 1, but uses D_FILE_2 (Hex: 6000-77FF, Dec: 24576-30719) and A_FILE_2 (Hex: 7800-7AFF, Dec: 30720-31487) instead.

	Mode 4: 'Ultra High Color Resoluton' mode uses D_FILE_1 to define pixel data (as with Mode 1)
		but holds attribute values in D_FILE_2 - this contains 8 times as much memory as A_FILE_1,
		allowing an attribute byte to be assigned to each row of pixels within each character.


from readme.txt in sna2jlo1.zip on nvg in /pub/sinclair/utils/ts2068

	a) All I/O ports in the TS2068 are fully decoded.  On the
	48K Spectrum, port #FE (reads keyboard) responds to all even
	port addresses.  This "feature" is not likely to cause any
	problems.

	b) On the TS2068, only D2 has a pull up resistor.  The Spectrum
	has pull ups on all data lines D0-D7.  So far I have found
	one game where this may be a problem:  Uridium.  This game
	doesn't let you select joystick or keyboard controls.  Instead
	it scans all joysticks and the keyboard.  My TS2068 does not
	have a Kempston joystick attached (port 31), so for example, when
	Uridium scans the Kempston joysticks, it reads a random collection
	of 0s and 1s on the data bus which are interpretted as joystick
	movements.  On the Spectrum, if a Kempston joystick were absent,
	only 1s would be read, indicating the joystick was in the
	neutral position.  The result is the spacecraft floats to the
	top of the screen.

	c) While the I/O ports the TS2068 and Spectrum have in common
	function identically, there are extra I/O ports on the TS2068
	used to control hardware the 48K Spectrum didn't have.  As long
	as games don't fiddle with ports that were never used on the
	original Spectrum, all should be fine.

	I have tested SNA2JLO on around 160 games.  I have found three
	that don't work:  Uridium (problem described above), Arkanoid
	(stalls at the beginning of level 1) and Academy (won't
	respond to keys).  That means your TS2068 can run about
	98% of the software out there.


some source.c:

	TS2068:
	Port 0xFF: set display mode:
	byte = %BIaaavvv

		Bits vvv: Video Mode:
		000 = Primary DFILE active		standard zxsp screen at 0x4000-0x5aff
		001 = Secondary DFILE active	standard zxsp screen at 0x6000-0x7aff
		010 = Extended Colour Mode		chars at 0x4000-0x57ff, colors at 0x6000-0x7aff
		110 = 64 column mode			columns 0,2,4,...62 from DFILE 1
										columns 1,3,5,...63 from DFILE 2
		other = unpredictable results

		Bits aaa: ink/paper selection for 64 column mode	(zxsp screen attributes in brackets)
		000 = Black/White		(56)
		001 = Blue/Yellow		(49)
		010 = Red/Cyan			(42)
		011 = Magenta/Green		(35)
		100 = Green/Magenta		(28)
		101 = Cyan/Red			(21)
		110 = Yellow/Blue		(14)
		111 = White/Black		(7)

		Bit6: If set disables the generation of the timer interrupt.
		Bit7: determines which bank to use: 0=DOCK, 1=EX-ROM.	(not present on TC2048)

	reading port_ff returns the last byte written to it.

http://8bit.yarek.pl/computer/zx.tc2048/index-de.html:

	In Timex Computer 2048 ULA is seen only in #FE port.
	Other ports used are #F4 (memory switching, not used in TC2048),
	#FF (screen mode, memory switching and interrupts disable) and Kempston Joystick interface.
	And the Kempston Joystick interface is the problem here, as is decoded only with line A5=0.
	So you have to read joystick in ports 0..31, 64..95, 128..160 and 192..224.
	This gives conflict with many external devices and there's no way to disable the input port without internal changes.
	(There are also #F5 and #F6 ports decoded for AY-3-8912, but since no device is connected to decoding lines, they don't make any problem.)
*/


#define io_addr		"----.----.1111.111-"


// Interrupt position:
// to be determined
//
#define cc_irpt_on	0
#define cc_irpt_off	64



UlaTc2048::UlaTc2048(Machine* m, isa_id id)
:
   UlaZxsp(m,id,io_addr,io_addr),
	byte_ff(0)
{}


void UlaTc2048::powerOn( /*t=0*/ int32 cc )
{
	byte_ff = 0;
	UlaZxsp::powerOn(cc);
	markVideoRam();
	border_color = 0;
	cpu->setInterrupt(cc_irpt_on, cc_irpt_off);
//	MmuTc2048Ptr(machine->mmu)->selectEXROM(0);		// get's it's own powerOn()
}

void UlaTc2048::reset(Time t, int32 cc )
{
	// the TC2048 and TC/TS2068 had no reset button
	// whether a reset via bus resetted the FF register is unknown
	// anyway, it had a power switch, so everybody did power-cycle the computer to reset.
	// so don't be picky here.
	byte_ff = 0;
	UlaZxsp::reset(t,cc);
	markVideoRam();
	border_color = 0;
	cpu->setInterrupt(cc_irpt_on, cc_irpt_off);
//	MmuTc2048Ptr(machine->mmu)->selectEXROM(0);		// get's it's own reset()
}


void UlaTc2048::setPortFF(uint8 byte)
{
	xlogline("xxx port FF = %2x",int(byte));

	byte_ff = byte;
	markVideoRam();
	border_color = byte&4 ? 8 + ((~byte>>3)&7) : ula_out_byte&7;
	cpu->setInterrupt( byte&0x40 ? 0x7fffffff : cc_irpt_on, cc_irpt_off );	// bit 6 disables the interrupt.
	MmuTc2048Ptr(machine->mmu)->selectEXROM(byte>>7);						// bit 7 selects bank to use on TS2068
}


void UlaTc2048::output( Time t, int32 cc, uint16 addr, uint8 byte )
{
	assert((addr&0xFE) == 0xFE);		// fully decoded

	if(addr&1) 	// addr = 0xFF => set Video Mode
	{
		if(byte==byte_ff) return;
		updateScreenUpToCycle(cc);
		record_ioinfo(cc,addr,byte);
		setPortFF(byte);
	}
	else		// addr = 0xFE => normal ULA access
	{
		UlaZxsp::output(t,cc,addr,byte);
	}
}


void UlaTc2048::input( Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask )
{
	assert((addr&0xFE) == 0xFE);		// fully decoded

	if(addr&1) 	// addr = 0xFF => get Video Mode
	{
		// reading 0xff on the Timex returns the last byte sent to the port:
		byte &= byte_ff;
		mask = 0xff;
	}
	else		// addr = 0xFE => normal ULA access
	{
		UlaZxsp::input(now,cc,addr,byte,mask);
	}
}


int32 UlaTc2048::updateScreenUpToCycle( int32 cc )
{
	int row = ccx / cc_per_line - lines_before_screen;
	int col = ccx % cc_per_line / cc_per_byte;				// in screen: 0*2 .. 15*2; else in side border

	assert( row>=0 );
	assert( row<lines_in_screen || ccx>=(1<<30) );
	assert( col<=30			   || ccx>=(1<<30) );
	assert( (col&1)==0          || ccx>=(1<<30) );

//		pro Scanline werden 32 Pärchen (64 Bytes) ausgegeben
//		32 column mode:  high byte = pixels;       low byte = attr
//		64 column mode:  high byte = left pixels;  low byte = right pixels

	bool twopages = byte_ff&6;	// 64 column mode OR high color mode read from both display files

	uint8* zap = attr_pixel + 2*( 32 * row + col );

b:	CoreByte* qp = video_ram + ( 32 * ((row&0xc0)+((row>>3)&0x7)+((row&7)<<3)) + col );
	CoreByte* qa = twopages ? qp + 0x2000 : video_ram + ( 32 * (192+row/8) + col );

a:	if( ccx>cc ) return ccx;

	*zap++ = *qp++; *zap++ = *qa++;		// pixels im 1. byte; attr im 2. byte
	*zap++ = *qp++; *zap++ = *qa++;

	ccx += 2*cc_per_byte;
	col += 2;
	if( col<32 ) goto a;

	ccx += cc_per_side_border;
	col  = 0;
	row += 1;
	if( row<lines_in_screen ) goto b;

	return ccx = 1<<30;
}


int32 UlaTc2048::doFrameFlyback( int32 )
{
	updateScreenUpToCycle(cc_frame_end);			// screen
	ccx = lines_before_screen*cc_per_line;			// update_screen_cc
	current_frame++;								// flash phase

	cpu->setInterrupt( byte_ff&0x40 ? 0x7fffffff : cc_irpt_on, cc_irpt_off ); // bit 6 disables the interrupt

	record_ioinfo(cc_frame_end,0xfe,0x00);			// for 60Hz models: remainder of screen is black
	if(ioinfo_count==ioinfo_size) grow_ioinfo();	// required by Renderer
	bool new_buffers_in_use = ScreenZxspPtr(screen)->ffb_or_vbi(ioinfo, ioinfo_count, attr_pixel, cc_screen_start,
											cc_per_side_border+128, getFlashPhase(), 90000/*cc_frame_end*/ );

	if(new_buffers_in_use)
	{
		std::swap(ioinfo, alt_ioinfo);
		std::swap(ioinfo_size, alt_ioinfo_size);
		std::swap(attr_pixel, alt_attr_pixel);
	}

	ioinfo_count = 0;
	record_ioinfo(0,0xfe,ula_out_byte);
	record_ioinfo(0,0xff,byte_ff);
	return cc_frame_end;							// cc_per_frame for last frame
}



void UlaTc2048::markVideoRam()
{
//	Port 0xFF: set display mode:
//  bits[2..0]:
//		000 = Primary DFILE active		standard zxsp screen at 0x4000-0x5aff
//		001 = Secondary DFILE active	standard zxsp screen at 0x6000-0x7aff
//		010 = Extended Colour Mode		chars at 0x4000-0x57ff, colors at 0x6000-0x7aff
//		110 = 64 column mode			columns 0,2,4,...62 from DFILE 1

	CoreByte* v = video_ram = machine->ram.getData();

	#define SET(A,SZ)	if(~v[A] & cpu_crtc) for( uint32 j=A, e=A+SZ; j<e; j++ ) v[j] |=  cpu_crtc;
	#define RES(A,SZ)	if( v[A] & cpu_crtc) for( uint32 j=A, e=A+SZ; j<e; j++ ) v[j] &= uint32(~cpu_crtc);

	if(!screen)
	{
		RES(0x0000,24*32*9);
		RES(0x2000,24*32*9);
	}
	else if(byte_ff&6)			// 64 column mode OR high color mode read from both display files
	{							// for simplicity also set attr bytes
		SET(0x0000,24*32*9);
		SET(0x2000,24*32*9);
	}
	else if(byte_ff & 1)		// display file 1:
	{
		video_ram = v + 0x2000;	// 0x6000
		SET(0x2000,24*32*9);
		RES(0x0000,24*32*9);
	}
	else						// display file 0:
	{
		SET(0x0000,24*32*9);
		RES(0x2000,24*32*9);
	}
}









































