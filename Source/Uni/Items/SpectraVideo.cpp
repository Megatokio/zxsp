// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SpectraVideo.h"
#include "Items/Z80/Z80options.h"
#include "Joystick.h" // physical joysticks
#include "Machine.h"
#include "RecentFilesMenu.h"
#include "Screen/Screen.h"
#include "Ula/Mmu.h"
#include "Ula/Ula.h"
#include "Ula/UlaZxsp.h"
#include "Z80/Z80.h"


/*	this class represents a SPECTRA video interface.

	kempston joystick:
		address:	%xxxx.xxxx.000x.xxxx
		data byte:	%000FUDLR active high		high bits always 0

	Display mode register:	((handled in SpectraVideo))
		address:	%0111.1111.1101.1111		in & out, fully decoded
												note: suppresses IORQ fur devices behind itself if this port is accessed
		data byte:	%FSDB.SCLL					F=full/half cell; S=Shadow bank; D=Display bank; B=Standard/Enhanced
   Border; S=Single/Double Byte color; C=Basic/Extra colors; LL=Line height

	RS232:
	Port 239:
		address		%xxx0.1xxx
		Output:		%111C.111R					C: 1 = CTS;  R: Comms_Out: 1 = RS232 Mode
		Input:		%xxxx.Dxxx					D: 1 = DTR

	Port 247:
		address:	%xxx1.0xxx
		Output:		%0000.000X					X: inverted data bit; 0=idle
		Input:		%Xxxx.xxxx					X: inverted data bit; 0=idle


	Display memory:
		4000-57FF	standard display pixels
		5800-5AFF	standard display attr

	Attributes:
			Line Mode	Single Byte Color	Double Byte Color
		%00 row			5800-5aff			5800-5aff + 5c00-5eff
		%01 quad		6000-65ff			6000-65ff + 6800-6dff
		%10	dual		6000-6bff			6000-6bff + 7000-7bff
		%11	single		6000-77ff			5800-7fff
*/


// RS232 RST8 hook codes:
// #define RS232_READ_BYTE			0x1d
// #define RS232_WRITE_BYTE  		0x1e
// #define RS232_OPEN_BIN_CHAN		0x34
// #define ZXIF1_CREATE_SYSVARS  	0x31

// ROM PAGING ADDRESSES
#define ROM_ERROR_HANDLER 0x0008 // byte at 0x0008 is read from internal rom, subsequent bytes from external rom
#define ROM_RESTORE		  0x0700 // byte at 0x0700 is read from external rom, subsequent bytes from internal rom
#define ROM_CLOSE_CHANNEL 0x1708 // byte at 0x1708 is read from internal rom, subsequent bytes from external rom


//	Display mode register:
//	address:	0x7FDF =
//				%0111.1111.1101.1111	in & out, fully decoded
//				suppresses IORQ for devices behind itself if this port is accessed
//	data byte:	%FSDB.SCLL				F=full/half cell; S=Shadow bank; D=Display bank; B=Standard/Enhanced Border;


// Display mode register bit masks:
#define HALFCELLMODE_MASK	 0x80
#define SHADOW_BANK_MASK	 0x40
#define DISPLAY_BANK_MASK	 0x20
#define ENHANCED_BORDER_MASK 0x10
#define TWO_BYTE_COLOUR_MASK 0x08
#define EXTRA_COLOURS_MASK	 0x04
#define LINE_HEIGHT_MASK	 0x03


// Paul 2013-06-11:
// Wenn der Displaymode-Schalter auf AUS steht, wird der Inhalt des Registers 0x7FDF auf 0x00 gezwungen.
// Ein darin gespeicherter Wert wird gelöscht und das Register kann nicht geschrieben werden.
// Nach dem Wieder-Einschalten enthält das Register den Wert 0x00.


/*	Paul 2016-02-27

	SPECTRA 128k:
	can be attached to +3 with adapter, but does not support port_1ffd

	port $7FFD Bit 3 = 128 Screen				decoded: %0111.----.----.--0-
	port $7FDF Bit 5 = SPECTRA Display bank		decoded: %0111.1111.1101.1111
	port $7FDF Bit 6 = SPECTRA Shadow bank

	video_ram =
		SPECTRA bank ($7FFD bit 3) xor ($7FDF bit 5)

	Writing to bank $4000 goes to
		SPECTRA bank $7FDF bit 6

	If page#5 or page#7 is paged-in at $C000, then
		writing to bank $C000 goes to
		SPECTRA bank (7FFD bit 1) xor ($7FDF bit 6)

	Paul 2016-03-01

	port 7FFD:
	zx128k:		"0---.----.----.--0-"
	+3/+2A:		"01--.----.----.--0-"
	Spectra128:	"0111.----.----.--0-"		difference confirmed


kio	 >	I had some thoughts on the SPECTRA+128. You can use it with a 48k Specci and get a non-contended screen.
paul	No. A switch on the interface selects 48K or 128K mode.
		Only in 128K mode will the interface respond to writes to the 128 IO display port.
		So on a 48k the interface only responds to writes to $4000 and so is contended.

kio	 >	I assume that the SPECTRA has no problems with a wait cycle free video page.
paul	SPECTRA has to mirror writes to the video RAM and so cannot be generating its SCART picture
		if the Spectrum is writing to video RAM as it must always mirror such writes.
		To ensure SPECTRA is always ready to mirror video RAM writes, it must generate its picture
		at exactly the same time as the ULA is generating its picture.

kio	 >	On a +3 you could use ram-only mode with non-contended ram pages 0,1,2,3 and still have video output!
paul	---> SPECTRA cannot mirror a non-contended RAM bank. <---

kio	 >	But what i am missing is a 512-pixel mode or, in general, TC2048 compatible screen modes.
paul	I could only fit in the extra logic for SPECTRA+128 because I needed less logic
		to synchronise to the CLK signal rather than the Y video signal.

kio	 >	i assume (remember?), that the SPECTRA just synchronizes it's screen with the ULA interrupt. right?
paul	Yes, SPECTRA synchronises its frames using the INT interrupt signal, exactly as the ULA does.

kio	 >	Is there a difference in screen timing between the SPECTRA and the SPECTRA+128?
paul	Yes and no. In 48K mode they are the same, in 128K mode they are different.

kio	 >	Is the video screen timed to match 224 CPU cycles per row as for the ZX Spectrum 48k?
paul	224 clock cycles in 48K mode and 228 clock cycles in 128K mode.

kio	 >	How many scan lines are displayed above the screen? I believe 64?
paul	(in essence): it replicates the timing of a zxsp 48k or a zx128k.
*/


//	%----.----.000-.----	in/---	Kempston joystick
//	%----.----.---0.1---	in/out	RS232
//	%----.----.---1.0---	in/out	RS232
//  %0111.1111.1101.1111	in/out	Display mode: color mode, shadow bank, display bank
//  %0111.----.----.--0-	--/out	ZX Spectrum 128k memory at $C000 and video display page
//	%----.----.----.---0	--/out	ZX Spectrum ULA border color
//
#define o_addr "----.----.----.----"
#define i_addr "----.----.----.----"


#define BYTES_PER_OCTET 3 // number of bytes stored per pixel octet for the renderer
#define CC_PER_BYTE		4 // ula cycles per pixel block  ((2 bytes == 8 pixel))


SpectraVideo::~SpectraVideo()
{
	assert(isMainThread());
	assert(machine->is_locked());

	xlogIn("~SpectraVideoInterface");

	ejectRom();

	delete[] attr_pixel;
	delete[] alt_attr_pixel;
	delete[] alt_ioinfo;

	machine->removeOverlay(overlay);
}


#define IOSZ 100


SpectraVideo::SpectraVideo(Machine* m, uint dip_switches) :
	Crtc(m, isa_SpectraVideo, isa_SpectraVideo, external, o_addr, i_addr), has_port_7ffd(machine->mmu->hasPort7ffd()),
	new_video_modes_enabled(dip_switches & EnableNewVideoModes),
	// port_7fdf(0),
	// port_7ffd(0),
	// shadowram_ever_used(no),
	shadowram(new Memory(m, "SPECTRA Video Ram", 0x8000)), joystick(nullptr), overlay(nullptr),
	// port_254(0),
	// port_239(0),
	// port_247(0),
	rs232_enabled(dip_switches & EnableRs232), joystick_enabled(dip_switches & EnableJoystick),
	if1_rom_hooks_enabled(dip_switches & EnableIf1RomHooks), rom(nullptr), filepath(nullptr), own_romdis_state(false),
	// current_frame(0),
	// ccx(0),
	attr_pixel(new uint8[32 * 24 * 8 * BYTES_PER_OCTET]),	  // transfer buffers -> screen
	alt_attr_pixel(new uint8[32 * 24 * 8 * BYTES_PER_OCTET]), // swap buffer
	alt_ioinfo_size(IOSZ), alt_ioinfo(new IoInfo[IOSZ + 1]), cc_per_side_border(cc_per_line - 32 * cc_per_byte),
	cc_frame_end(lines_per_frame * cc_per_line), cc_screen_start(lines_before_screen * cc_per_line)
{
	assert(machine->isA(isa_MachineZxsp));

	video_ram = &shadowram[0];
	if (joystick_enabled) insertJoystick(usb_joystick0);
}


void SpectraVideo::powerOn(int32 cc)
{
	Crtc::powerOn(cc);
	assert(ioinfo_count == 0);

	current_frame		= 0;
	ccx					= cc_per_line * lines_before_screen;
	shadowram_ever_used = no;
	setup_timing();
	_reset();
}


void SpectraVideo::setup_timing()
{
	UlaZxsp* ula = UlaZxspPtr(machine->ula);

	assert(cc_per_byte == 4);						// ula cycles per crtc address increment (= 8 pixels)
	cc_per_line			= ula->cc_per_line;			// 48k: 224, +128k: 228
	lines_before_screen = ula->lines_before_screen; // 48k: 64, +128k: 63
	assert(lines_in_screen == 192);					// 192
	lines_after_screen = ula->lines_after_screen;	// 56
	lines_per_frame	   = ula->lines_per_frame;		// 312 / 311

	cc_per_side_border = ula->cc_per_side_border; // cc_per_line - 32 * cc_per_byte;
	cc_frame_end	   = ula->cc_frame_end;		  // lines_per_frame * cc_per_line;
	cc_screen_start	   = ula->cc_screen_start;	  // lines_before_screen*cc_per_line;
}


void SpectraVideo::_reset()
{
	port_7fdf = 0;
	port_7ffd = 0;
	port_254  = 0;
	port_239  = 0; // ?
	port_247  = 0; // ?

	shadowram_ever_used = no;
	map_shadow_ram();
	init_rom();
	markVideoRam();
}


void SpectraVideo::reset(Time t, int32 cc)
{
	Crtc::reset(t, cc);

	updateScreenUpToCycle(cc);
	record_ioinfo(cc, 0x7FDF, 0);
	record_ioinfo(cc, 0xFFFE, 0);

	_reset();
}


void SpectraVideo::input(Time /*t*/, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask)
{
	//	%----.----.000-.----	in/---	Kempston joystick
	//	%----.----.---0.1---	in/out	RS232
	//	%----.----.---1.0---	in/out	RS232
	//  %0111.1111.1101.1111	in/out	Display mode: color mode, shadow bank, display bank

	// kempston interface:
	if ((addr & 0x00E0) == 0 && joystick_enabled)
	{
		// Input: %000FUDLR  active high
		mask = 0xff;
		byte &= machine == front_machine ? joystick->getState(yes) : 0x00;
	}

	// RS232 port 239:
	if ((addr & 0x18) == 0x08 && rs232_enabled)
	{
		// Input:		%xxxx.Dxxx					D: 1 = DTR
		logline("SpectraVideo: IN(239): TODO"); // TODO
		byte		  = 0;
		mask		  = 0xff;
		rs232_enabled = no;
	}

	// RS232 port 247:
	if ((addr & 0x18) == 0x10 && rs232_enabled)
	{
		// Input:		%Xxxx.xxxx					X: inverted data bit; 0=idle
		logline("SpectraVideo: IN(247): TODO"); // TODO
		byte		  = 0;
		mask		  = 0xff;
		rs232_enabled = no;
	}

	if (addr == 0x7FDF && new_video_modes_enabled)
	{
		byte &= port_7fdf;
		mask = 0xff;
	}
}


void SpectraVideo::output(Time t, int32 cc, uint16 addr, uint8 byte)
{
	//	%----.----.---0.1---	in/out	RS232
	//	%----.----.---1.0---	in/out	RS232
	//  %0111.1111.1101.1111	in/out	Display mode: color mode, shadow bank, display bank
	//  %0111.----.----.--0-	--/out	ZX Spectrum 128k memory at $C000 and video display page
	//	%----.----.----.---0	--/out	ZX Spectrum ULA Border color

	if (~addr & 1) // ULA border
	{
		uint x	 = byte ^ port_254;
		port_254 = byte;
		if (x & 0xE7) record_ioinfo(cc, addr, byte);
	}

	if ((addr & 0x3A) == 0x3A) return;				// quick exit test: at least one bit must be 0
	if ((addr & 0x18) == 0x08) setPort239(t, byte); // RS232
	if ((addr & 0x18) == 0x10) setPort247(t, byte); // RS232

	// ZX Spectrum +128k memory mode register:
	if ((addr & 0xF002) == 0x7000)
	{
		if (has_port_7ffd && !mmu_is_locked() && byte != port_7ffd)
		{
			uint x	  = port_7ffd ^ byte;
			port_7ffd = byte;
			//	if(byte&0x0a) shadowram_ever_used = yes;	// page 7 either accessed or displayed

			if (x & 0x08) // video output page changed
			{
				updateScreenUpToCycle(cc);
				markVideoRam();
			}
			if (x & 0x07)
			{
				map_shadow_ram(); // ram mapped at $C000 changed
			}
		}
	}

	// SPECTRA video mode register:
	else if (addr == 0x7FDF && new_video_modes_enabled && byte != port_7fdf)
	{
		updateScreenUpToCycle(cc);
		record_ioinfo(cc, 0x7FDF, byte);
		uint x	  = byte ^ port_7fdf;
		port_7fdf = byte;
		if (byte & (SHADOW_BANK_MASK | DISPLAY_BANK_MASK)) shadowram_ever_used = yes;
		if (x & (DISPLAY_BANK_MASK + TWO_BYTE_COLOUR_MASK + LINE_HEIGHT_MASK)) markVideoRam();
		if (x & SHADOW_BANK_MASK) map_shadow_ram();
	}
}


/*	Output to port 239: RS232
	byte = %111C.111R
		   C: 1 = CTS
		   R: Comms_Out: 1 = RS232 Mode
*/
void SpectraVideo::setPort239(Time, uint8 byte)
{
	port_239 = byte;
	if (rs232_enabled)
	{
		logline("SpectraVideo: OUT(239): TODO");
		rs232_enabled = false;
	}
}


/*	Output to port 247: RS232
	byte = %0000.000X
		   X: inverted data bit; 0=idle
*/
void SpectraVideo::setPort247(Time, uint8 byte)
{
	port_247 = byte;
	if (rs232_enabled)
	{
		logline("SpectraVideo: OUT(247): TODO");
		rs232_enabled = false;
	}
}


void SpectraVideo::setBorderColor(uint8 byte)
{
	if (!new_video_modes_enabled) byte &= 7;
	port_254 = byte;
	// updateScreenUpToCycle(cc);
	record_ioinfo(machine->current_cc(), 0xFFFE, byte);
}


/*	set video mode
	at current cc
	only if new_video_modes_enabled
*/
void SpectraVideo::setVideoMode(uint8 mode) { setPort7fdf(machine->current_cc(), mode); }


/*	set video mode
	only if new_video_modes_enabled
*/
void SpectraVideo::setPort7fdf(int32 cc, uint8 mode)
{
	if (new_video_modes_enabled)
	{
		updateScreenUpToCycle(cc);
		record_ioinfo(cc, 0x7FDF, mode);
		port_7fdf = mode;
		if (mode & (SHADOW_BANK_MASK | DISPLAY_BANK_MASK)) shadowram_ever_used = yes;
		map_shadow_ram();
		markVideoRam();
	}
}


/*	callback from Mmu128k:
 */
void SpectraVideo::setPort7ffd(uint8 byte)
{
	if (has_port_7ffd)
	{
		// updateScreenUpToCycle(cc);
		port_7ffd = byte;
		// if(byte&0x0a) shadowram_ever_used = yes;
		map_shadow_ram();
		markVideoRam();
	}
}


int32 SpectraVideo::updateScreenUpToCycle(int32 cc)
{
	int row = ccx / cc_per_line - lines_before_screen;
	int col = ccx % cc_per_line / cc_per_byte; // in screen: 0*2 .. 15*2; else in side border

	assert(row >= 0);
	assert(row < lines_in_screen || ccx >= (1 << 30));
	assert(col <= 30 || ccx >= (1 << 30));
	assert((col & 1) == 0 || ccx >= (1 << 30));

	uint8* zp = attr_pixel + 3 * (32 * row + col);

	do {
		uint lowbyte = ((row << 2) & 0xE0) | col;
		uint line	 = row & 7;	   // row inside char => L2,L1,L0
		uint quad	 = row & 0xC0; // screen block => A1,A0

		CoreByte *qp, *qa1, *qa2;

		qp = video_ram + (quad << 5) + (line << 8) + lowbyte;

		switch (port_7fdf & 3)
		{
		case 0: // 8-line mode
			qa1 = video_ram + 0x1800 + (quad << 2) + lowbyte;
			qa2 = qa1 + 0x0400;
			break;

		case 1: // 4-line mode
			qa1 = video_ram + 0x2000 + (quad << 3) + (line >> 2 << 8) + lowbyte;
			qa2 = qa1 + 0x0800;
			break;

		case 2: // 2-line mode
			qa1 = video_ram + 0x2000 + (quad << 4) + (line >> 1 << 8) + lowbyte;
			qa2 = qa1 + 0x1000;
			break;

		case 3: // 1-line mode

			if (port_7fdf & 8) // 1-line && 2-byte attr mode
			{
				if (row < 128) // upper or middle block
				{
					qa1 = video_ram + 0x2000 + (quad << 5) + (line << 8) + lowbyte;
					qa2 = qa1 + 0x1000;
				}
				else // lower screen block
				{
					qa1 = video_ram + 0x1800 + (line >> 1 << 8) + lowbyte;
					qa2 = qa1 + 0x400;
				}
			}
			else // 1-line && 1-byte attr mode
			{	 // => both attr bytes read from same address
				qa1 = video_ram + 0x2000 + (quad << 5) + (line << 8) + lowbyte;
				qa2 = qa1;
			}
			break;
		}

		do {
			if (ccx > cc) return ccx;

			*zp++ = *qp++;
			*zp++ = *qa1++;
			*zp++ = *qa2++; // 1st pixel octet:	pixels im 1. byte; attr im 2. byte, 2nd attr im 3. byte
			*zp++ = *qp++;
			*zp++ = *qa1++;
			*zp++ = *qa2++; // 2nd pixel octet: ""

			ccx += 2 * cc_per_byte;
			col += 2;
		}
		while (col < 32);

		ccx += cc_per_side_border;
		col = 0;
		row += 1;
	}
	while (row < lines_in_screen);

	return ccx = 1 << 30;
}


int32 SpectraVideo::doFrameFlyback(int32 /*cc*/) // called from runForSound()
{
	current_frame++; // flash phase

	if (screen)
	{
		updateScreenUpToCycle(cc_frame_end);	 // screen
		ccx = lines_before_screen * cc_per_line; // update_screen_cc

		record_ioinfo(cc_frame_end, 0xfe, 0x00); // for 60Hz models: remainder of screen is black
		bool new_buffers_in_use = ScreenZxspPtr(screen)->ffb_or_vbi(
			ioinfo,
			ioinfo_count,
			attr_pixel,
			cc_screen_start,
			cc_per_side_border + 128,
			get_flash_phase(),
			90000 /*cc_frame_end*/);

		if (new_buffers_in_use)
		{
			std::swap(ioinfo, alt_ioinfo);
			std::swap(ioinfo_size, alt_ioinfo_size);
			std::swap(attr_pixel, alt_attr_pixel);
		}
	}

	ioinfo_count = 0;
	record_ioinfo(0, 0xFFFE, port_254);
	record_ioinfo(0, 0x7FDF, port_7fdf);

	return cc_frame_end; // cc_per_frame for last frame
}


void SpectraVideo::drawVideoBeamIndicator(int32 cc) // called from runForSound()
{
	if (screen)
	{
		updateScreenUpToCycle(cc);
		bool new_buffers_in_use = ScreenZxspPtr(screen)->ffb_or_vbi(
			ioinfo, ioinfo_count, attr_pixel, cc_screen_start, cc_per_side_border + 128, get_flash_phase(), cc);

		if (new_buffers_in_use)
		{
			std::swap(ioinfo, alt_ioinfo);
			std::swap(ioinfo_size, alt_ioinfo_size);
			std::swap(attr_pixel, alt_attr_pixel);

			uint32 n = 32 * 24 * 8;
			if (ccx != 1 << 30)
			{
				int row = ccx / cc_per_line - lines_before_screen;
				int col = ccx % cc_per_line / cc_per_byte;
				n		= 32 * row + col;
				assert(n <= 32 * 24 * 8);
			}

			memcpy(ioinfo, alt_ioinfo, ioinfo_count * sizeof(IoInfo));
			memcpy(attr_pixel, alt_attr_pixel, BYTES_PER_OCTET * n);
		}
	}
}


/*	enable or disable the new video modes
 */
void SpectraVideo::enableNewVideoModes(bool f)
{
	assert(is_locked());

	if (new_video_modes_enabled == f) return;
	if (port_7fdf == 0)
	{
		new_video_modes_enabled = f;
		return;
	}

	assert(!f); // kann jetzt nur noch Ausschalten sein

	new_video_modes_enabled = false;
	port_7fdf				= 0;
	map_shadow_ram();
	markVideoRam();
	//	set_video_mode(machine->current_cc(), 0);
}

void SpectraVideo::romCS(bool f)
{
	if (f == romdis_in) return;
	romdis_in = f;

	if (!f && rom.ptr())
		activateRom(); // also emits romCS
	else
		prev()->romCS(f || own_romdis_state);
}

void SpectraVideo::setRS232Enabled(bool f) { rs232_enabled = f; }

void SpectraVideo::setJoystickEnabled(bool f) { joystick_enabled = f; }

void SpectraVideo::setIF1RomHooksEnabled(bool f)
{
	assert(is_locked());

	if (if1_rom_hooks_enabled == f) return;
	if1_rom_hooks_enabled = f;
	init_rom();
}

void SpectraVideo::insertRom(cstr path)
{
	assert(is_locked());

	ejectRom();

	FD	   fd(path, 'r');
	uint32 sz = fd.file_size();
	if (sz > 0x4000) sz = 0x4000;
	//	rom.grow(0x4000);
	rom = new Memory(machine, basename_from_path(path), 0x4000);

	read_mem(fd, rom.getData(), sz);
	if (sz <= 0x2000) memcpy(&rom[0x2000], &rom[0], 0x2000); // miror 8k roms
	filepath = newcopy(path);

	init_rom();
	addRecentFile(gui::RecentIf2Roms, path);
	addRecentFile(gui::RecentFiles, path);
}

void SpectraVideo::ejectRom()
{
	assert(is_locked());

	delete[] filepath;
	filepath = nullptr;
	if (rom.ptr() == 0) return;

	deactivateRom();
	deactivate_hooks();
	rom = nullptr;
}

uint8 SpectraVideo::handleRomPatch(uint16 pc, uint8 opcode)
{
	if (if1_rom_hooks_enabled && rom.ptr())
	{
		if (own_romdis_state) // in SPECTRA rom:
		{
			if (pc == ROM_RESTORE)
			{
				deactivateRom();
				return opcode; // rom not switched immediately
			}
		}
		else // in Machine's rom:
		{
			if ((pc == ROM_ERROR_HANDLER || pc == ROM_CLOSE_CHANNEL))
			{
				activateRom();
				return opcode; // rom not switched immediately
			}
		}
	}

	return prev()->handleRomPatch(pc, opcode);
}

void SpectraVideo::init_rom()
{
	if (rom.ptr())
	{
		if (if1_rom_hooks_enabled)
		{
			activate_hooks();
			deactivateRom();
		}
		else
		{
			deactivate_hooks();
			activateRom();
		}
	}
	else
		own_romdis_state = false;
}

void SpectraVideo::activateRom()
{
	if (rom.ptr() == nullptr) return;
	own_romdis_state = true;
	if (romdis_in) return;

	assert(rom.count() == 0x4000);

	prev()->romCS(true);
	machine->cpu->mapRom(0 /*addr*/, 0x4000 /*size*/, rom.getData(), nullptr, 0);
}

void SpectraVideo::deactivateRom()
{
	own_romdis_state = false;
	if (romdis_in) return;
	if (rom.ptr() == nullptr) return;
	prev()->romCS(false);

	//	if(machine->cpu->rdPtr(0)==rom.getData()) machine->cpu->unmapRom(0/*addr*/,0x4000/*size*/);
}

void SpectraVideo::activate_hooks()
{
	assert(rom.ptr());

	rom[ROM_RESTORE] |= cpu_patch;
	machine->rom[ROM_ERROR_HANDLER] |= cpu_patch;
	machine->rom[ROM_CLOSE_CHANNEL] |= cpu_patch;
}

void SpectraVideo::deactivate_hooks()
{
	assert(rom.ptr());

	rom[ROM_RESTORE] &= ~cpu_patch;
	machine->rom[ROM_ERROR_HANDLER] &= ~cpu_patch;
	machine->rom[ROM_CLOSE_CHANNEL] &= ~cpu_patch;
}


/*	Map/unmap shadow ram

	note: if new video modes are disabled, then port_7fdf must be 0
	note: if this is not a SPECTRA 128k, then port_7ffd must be 0
	note: caller should test beforehand whether video mode or video page actually changed
*/
void SpectraVideo::map_shadow_ram()
{
	// Writing to address $4000 goes to SPECTRA bank $7FDF bit 6
	machine->cpu->mapWom2(0x4000, 0x4000, &shadowram[(port_7fdf & 0x40u) << 8]);

	// If the SPECTRA 128k believes that page#5 or page#7 is paged-in at $C000,
	// then writing to cpu address $C000 also goes to
	// SPECTRA bank (port_7FFD bit 1) xor (port_7FDF bit 6)
	// note: if uncontended ram is mapped to $C000 in ram-only mode on a +3/+2A,
	// then the real-world SPECTRA will not work properly. This is not reproduced.
	if (has_port_7ffd)
	{
		assert(machine->ram.count() >= 128 kB);

		if ((port_7ffd & 5) == 5) // video page paged in
		{
			uint32 spectra_page = ((port_7ffd << 13) ^ (port_7fdf << 8)) & 0x4000;
			machine->cpu->mapWom2(0xC000, 0x4000, &shadowram[spectra_page]);
		}
		else // non-video page paged in
		{
			machine->cpu->unmapWom2(0xC000, 0x4000);
		}
	}
}


/*	Update video_ram and CRTC flag bits in SPECTRA ram:
	clear and set crtc flags in SPECTRA ram according to current display mode in port_7fdf
	set video_ram according to bits in port_7fdf and port_7ffd
	note: if new video modes are disabled, then port_7fdf must be 0
	note: if this is not a SPECTRA 128k, then port_7ffd must be 0
	note: caller should test beforehand whether video mode or video page actually changed
*/
void SpectraVideo::markVideoRam()
{
	//							 1B,8L	 1B,4L	 1B,2L	 1B,1L				2B,8L	2B,4L	2B,2L	2B,1L
	static const uint a1[12] = {0x1800, 0x2000, 0x2000, 0x2000, 0, 0, 0, 0, 0x1800, 0x2000, 0x2000, 0x1800};
	static const uint e1[12] = {0x1B00, 0x2600, 0x2C00, 0x3800, 0, 0, 0, 0, 0x1B00, 0x2600, 0x2C00, 0x1B00};
	static const uint a2[12] = {0x1C00, 0x2800, 0x3000, 0x2000, 0, 0, 0, 0, 0x1C00, 0x2800, 0x3000, 0x2000};
	static const uint e2[12] = {0x1F00, 0x2E00, 0x3C00, 0x3800, 0, 0, 0, 0, 0x1F00, 0x2E00, 0x3C00, 0x4000};

	CoreByte *v = &shadowram[0], *a, *e;
	// videoram = SPECTRA bank ($7FFD bit 3) xor ($7FDF bit 5)
	video_ram = v + (((port_7ffd << 11) ^ (port_7fdf << 9)) & 0x4000); // video ram pointer
	uint i	  = port_7fdf & (TWO_BYTE_COLOUR_MASK + LINE_HEIGHT_MASK); // 0b00001011

	e = &video_ram[0x0000];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (*a & cpu_crtc)
			while (a < v) *a++ &= ~cpu_crtc;
	}
	e = &video_ram[0x1800];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (~*a & cpu_crtc)
			while (a < v) *a++ |= cpu_crtc;
	}
	e = &video_ram[a1[i]];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (*a & cpu_crtc)
			while (a < v) *a++ &= ~cpu_crtc;
	}
	e = &video_ram[e1[i]];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (~*a & cpu_crtc)
			while (a < v) *a++ |= cpu_crtc;
	}
	e = &video_ram[a2[i]];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (*a & cpu_crtc)
			while (a < v) *a++ &= ~cpu_crtc;
	}
	e = &video_ram[e2[i]];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (~*a & cpu_crtc)
			while (a < v) *a++ |= cpu_crtc;
	}
	e = &shadowram[0x7FFF];
	while (v < e)
	{
		a = v;
		v += 0x100;
		if (*a & cpu_crtc)
			while (a < v) *a++ &= ~cpu_crtc;
	}
}


void SpectraVideo::insertJoystick(int id) volatile
{
	if (joystick == joysticks[id]) return;

	if (overlay)
	{
		machine->removeOverlay(overlay);
		overlay = nullptr;
	}
	joystick = joysticks[id];
	if (id != no_joystick) overlay = machine->addOverlay(joystick, "K", gui::Overlay::TopRight);
}
