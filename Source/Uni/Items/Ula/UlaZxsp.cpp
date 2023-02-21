// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	waitstates for any i/o:
	• waitstates are added for contended memory access
	• waitstates are added for ula access (i/o with A0=0)
	• waitstates are added for non-ula i/o with address matching contended memory page
	  • not for +3
	  • on +128, this applies to $4000++ and $C000++ if contended memory is currently paged in
	• waitstates are inserted as for memory, if the i/o is "contended"

	•	A14,A15 =   |       |
		cont. page? | bit 0 | Contention pattern
		------------+-------+-------------------
			 No     |   0   | N:1, C:3
			Yes     |   0   | C:1, C:3
			 No     |   1   | N:4
			Yes     |   1   | C:1, C:1, C:1, C:1
*/

#include "UlaZxsp.h"
#include "Dsp.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/Screen/Screen.h"
#include "TapeRecorder.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"


#define io_addr "----.----.----.---0" // übliche Adresse: $FE     BESTÄTIGT


#define KEYBOARD_IN_MASK 0x1F
#define EAR_IN_MASK		 0x40

#define BORDER_OUT_MASK 0x07
#define MIC_OUT_MASK	0x08
#define EAR_OUT_MASK	0x10


#define IOSZ 100


UlaZxsp::UlaZxsp(Machine* m, isa_id id, cstr oaddr, cstr iaddr) :
	Ula(m, id, oaddr, iaddr),
	cc_per_side_border(), // Zeit für Seitenborder+Strahlrücklauf
	cc_waitmap_start(),	  // Ab wann die Waitmap benutzt werden muss
	cc_screen_start(),	  // Erster cc für einen CRT Backcall
	cc_waitmap_end(),	  // Ab wann nicht mehr
	cc_frame_end(),		  // Total cpu clocks per Frame
	waitmap_size(0),
	cpu(m->cpu),
	ram(m->ram),
	// current_frame(0),			// counter, used for flash phase
	// ccx(0),					// next cc for reading from video_ram
	attr_pixel(newAttrPixelArray()),	 // specci screen attribute and pixel tupels
	alt_attr_pixel(newAttrPixelArray()), // alternate data set
	alt_ioinfo(new IoInfo[IOSZ + 1]),
	alt_ioinfo_size(IOSZ),
	earin_threshold_mic_lo(info->earin_threshold_mic_lo),
	earin_threshold_mic_hi(info->earin_threshold_mic_hi)
{
	xlogIn("new UlaZxsp");

	assert(sizeof(waitmap) == MAX_BYTES_PER_LINE * cc_per_byte);
	assert(screen && screen->isA(isa_ScreenZxsp));
	assert(video_ram == ram.getData()); // current video ram
	m->cpu_options |= cpu_floating_bus;
}


UlaZxsp::UlaZxsp(Machine* m) : UlaZxsp(m, isa_UlaZxsp, io_addr, io_addr) {}


UlaTk90x::UlaTk90x(Machine* m, bool is60hz) : UlaZxsp(m, isa_UlaTk90x, io_addr, io_addr) { UlaTk90x::set60Hz(is60hz); }


UlaZxsp::~UlaZxsp()
{
	xlogIn("~UlaZxsp");
	delete[] attr_pixel;
	delete[] alt_attr_pixel;
	delete[] alt_ioinfo;
}


void UlaZxsp::powerOn(int32 cc)
{
	xlogIn("UlaZxsp:Init");

	current_frame = 0;
	ccx			  = cc_per_line * lines_before_screen;

	// ear/mic:
	Ula::powerOn(cc);

	// item refs:
	cpu		  = machine->cpu;
	ram		  = machine->ram;
	video_ram = ram.getData();
	assert(!screen || screen->isA(isa_ScreenZxsp));

	// Setup CRTC:
	setupTiming();
	markVideoRam();
}


/*	setup timing parameters
	in:	cc_per_line
		lines_before_screen
		lines_in_screen
		lines_after_screen
*/
void UlaZxsp::setupTiming()
{
	// validate settings.:
	static_assert(cc_per_byte == 4, "cc_per_byte must be 4");
	assert(uint(cc_per_line) <= NELEM(waitmap));
	assert(cc_per_line >= MIN_BYTES_PER_LINE * cc_per_byte);
	assert(lines_before_screen >= MIN_LINES_BEFORE_SCREEN);
	assert(lines_before_screen <= MAX_LINES_BEFORE_SCREEN);
	assert(lines_after_screen >= MIN_LINES_AFTER_SCREEN);
	assert(lines_after_screen <= MAX_LINES_AFTER_SCREEN);
	assert(lines_in_screen == 192);

	// Abgeleitete Timing-Werte
	lines_per_frame	   = lines_before_screen + lines_in_screen + lines_after_screen;
	cc_per_side_border = cc_per_line - 32 * cc_per_byte;
	cc_screen_start	   = lines_before_screen * cc_per_line;
	cc_frame_end	   = lines_per_frame * cc_per_line;

	ccx = max(ccx, cc_screen_start); // ccx (cc of next video byte read) must be ≥ screen start!

	// Waitmap:
	uint8 wm = info->waitmap; // e.g. wm_bits = %11111100  -->  bu[8] = {6,5,4,3,2,1,0,0}
	if (wm)
	{
		// von wann bis wann soll die cpu die waitmap benutzen?
		cc_waitmap_start = cc_screen_start - 4 /*etwas früher*/;
		cc_waitmap_end	 = cc_screen_start + lines_in_screen * cc_per_line;

		// Waitmap für 16 * 16-Pixel-Blocks auswalzen:
		waitmap_size = cc_per_line;
		memset(waitmap, 0, sizeof(waitmap));
		for (int d = 0, i = 7; i >= 0; i--)
		{
			waitmap[i] = wm & 1 ? ++d : (d = 0);
			wm >>= 1;
		}
		for (int i = 1; i < 16; i++) { memcpy(waitmap + i * 8, waitmap, 8); }
	}
	else
	{
		waitmap_size	 = 0;
		cc_waitmap_start = cc_waitmap_end = cc_frame_end;
	}

	// Info
	xlogline("UlaZxsp:setup_timing:");
	xlogline("+ lines_before_screen = %i", lines_before_screen);
	xlogline("  lines_in_screen     = %i", lines_in_screen);
	xlogline("  lines_after_screen  = %i", lines_after_screen);

	xlogline("+ cc_per_line         = %i", cc_per_line);
	xlogline("  cc_per_side_border  = %i", cc_per_side_border);

	xlogline("+ cc_waitmap_start    = %i", cc_waitmap_start);
	xlogline("  cc_screen_start     = %i", cc_screen_start);
	xlogline("  cc_waitmap_end      = %i", cc_waitmap_end);
	xlogline("  cc_frame_end        = %i", cc_frame_end);
}


/*	set or remove the cpu_crtc flag on my video ram
	normally the flags are only ever set,
	but when the Spectra video interface is connected, these bits should be cleared
*/
void UlaZxsp::markVideoRam()
{
	CoreByte* v = video_ram = ram.getData();

	if (screen) // set flag:
	{
		if (~v[0] & cpu_crtc)
			for (uint32 j = 0, e = 6912; j < e; j++) v[j] |= cpu_crtc;
	}
	else // remove flag:
	{
		if (v[0] & cpu_crtc)
			for (uint32 j = 0, e = 6912; j < e; j++) v[j] &= ~cpu_crtc;
	}
}


/*	In an i/o instruction add wait cycles (if any)
	called from machine.input() and machine.output() before i/o handlers of all items
*/
int32 UlaZxsp::addWaitCycles(int32 cc, uint16 addr) const volatile
{
	if (cc < cc_waitmap_start || cc >= cc_waitmap_end) return cc; // not in screen

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

	// page 1 is contended:
	bool contended = (addr & 0xc000) == 0x4000;

	if (addr & 0x0001) // ULA not accessed:
	{
		if (contended)
		{
			// access to address which looks like a screen memory access:
			cc += waitmap[(cc - 1) % cc_per_line]; // -1, 0, +1, +2  --> see BorderBarGenerator
			cc += waitmap[(cc + 0) % cc_per_line];
			cc += waitmap[(cc + 1) % cc_per_line];
			cc += waitmap[(cc + 2) % cc_per_line];
			cpu->setCpuCycle(cc);
		}
	}
	else // ULA accessed:
	{
		if (contended) cc += waitmap[(cc - 1) % cc_per_line];
		cc += waitmap[(cc + 0) % cc_per_line];
		cpu->setCpuCycle(cc);
	}
	return cc;
}


/*	get floating bus byte at cycle
	called by machine.input() if some bits in the input byte are still unset.
	mostly this is 0xFF.
*/
uint8 UlaZxsp::getFloatingBusByte(int32 cc)
{
	// ULA won't stop -> read mixture of 0xff (idle bus) and attribute byte
	// 2006-10-06 kio: issue 2: reads only attribute bytes, never pixel bytes.
	// TODO: at which clock cycle inside each 2-characters pixel packet
	//	     is which attribute byte read, and when $ff

	cc -= cc_screen_start;
	if (cc < 0) return 0xff; // above screen

	int row = cc / cc_per_line;
	if (row >= lines_in_screen) return 0xff; // below screen

	int col = (cc % cc_per_line) / cc_per_byte;
	if (col >= 32) return 0xff; // in side border

	// return attribute byte:
	return video_ram[24 * 8 * 32 + 32 * (row / 8) + col];
}


/* ---------------------------------------------------------------
				Output to ULA
--------------------------------------------------------------- */

void UlaZxsp::output(Time now, int32 cc, uint16 addr, uint8 byte)
{
	assert(~addr & 1);

	//	The lowest three bits specify the border colour;
	//	a zero in bit 3 activates the MIC output,
	//	whilst a one in bit 4 activates the EAR output and the internal speaker.
	//	However, the EAR and MIC sockets are connected only by resistors,
	//	so activating one activates the other;
	//	the EAR is generally used for output as it produces a louder sound.
	//	The upper two bits are unused.

	uint x		 = byte ^ ula_out_byte;
	ula_out_byte = byte;

	// --- TAPE ---
	if ((x & MIC_OUT_MASK) && machine->taperecorder->isRecording())
	{
		machine->taperecorder->output(cc, ~byte & MIC_OUT_MASK);
	}

	// --- BEEPER ---
	if (x & (MIC_OUT_MASK | EAR_OUT_MASK))
	{
		os::outputSamples(beeper_current_sample, beeper_last_sample_time, now);
		beeper_last_sample_time = now;

		uint bb = byte ^ MIC_OUT_MASK; // mic pin is low active
		beeper_current_sample =
			beeper_volume * -0.25 * ((bb & EAR_OUT_MASK) * 3 / EAR_OUT_MASK + (bb & MIC_OUT_MASK) * 1 / MIC_OUT_MASK);
	}

	// --- BORDER ---
	if (x & BORDER_OUT_MASK)
	{
		record_ioinfo(cc, addr, byte);
		border_color = byte & BORDER_OUT_MASK;
	}
}


/* ---------------------------------------------------------------
				Input from ULA
--------------------------------------------------------------- */

void UlaZxsp::input(Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	assert(~addr & 1);

	// bits 0-4 come from the keyboard
	// bits 5+7 are always 1
	// bit  6	is the signal from the ear socket messed up with ear+mic outputs
	mask = 0xff; // to be tested

	// --- keyboard ---

	byte &= readKeyboard(addr);

	// --- ear in ---

	if (machine->taperecorder->isPlaying())
	{
		if (!machine->taperecorder->input(cc)) byte &= ~EAR_IN_MASK;
		return;
	}

	Sample threshold = ula_out_byte & MIC_OUT_MASK ? earin_threshold_mic_hi : earin_threshold_mic_lo;

	if (machine->audio_in_enabled)
	{
		if (ula_out_byte & EAR_OUT_MASK) // EAR_OUT==1
		{
			// SINCLAIR ULAs:
			// EAR_OUT = 1 forces EAR_IN = 1
			// ULA EAR_IN is **probably** always "1"  =>  nothing to do
			// verified 2006-06-29 for issue 2, 4A, and 6U (128K)
		}
		else // EAR_OUT==0
		{
			// signal from EAR input is read into bit 6.
			// threshold level depends on state of EAR output bit and PCB issue/model!
			// TODO: simulate capacitor

			uint32 a = uint32(now * samples_per_second);
			if (a >= DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING)
			{
				assert(int32(a) >= 0);
				showAlert("Sample input beyond dsp buffer: +%i\n", int(a - DSP_SAMPLES_PER_BUFFER));
				if (0.0 < threshold) byte &= ~EAR_IN_MASK;
			}
			else
			{
				if (os::audio_in_buffer[a] < threshold) byte &= ~EAR_IN_MASK;
			}
		}
	}
	else if (0.0 < threshold) byte &= ~EAR_IN_MASK;
}


int32 UlaZxsp::updateScreenUpToCycle(int32 cc)
{
	int row = ccx / cc_per_line - lines_before_screen;
	int col = ccx % cc_per_line / cc_per_byte; // in screen: 0*2 .. 15*2; else in side border

	assert(row >= 0);
	assert(row < lines_in_screen || ccx >= (1 << 30));
	assert(col <= 30 || ccx >= (1 << 30));
	assert((col & 1) == 0 || ccx >= (1 << 30));

	uint8* zp = attr_pixel + 2 * (32 * row + col);

	do {
		CoreByte* qp = video_ram + (32 * ((row & 0xc0) + ((row >> 3) & 0x7) + ((row & 7) << 3)) + col);
		CoreByte* qa = video_ram + (32 * (192 + row / 8) + col);

		do {
			if (ccx > cc) return ccx;

			*zp++ = *qp++;
			*zp++ = *qa++; // pixels im 1. byte; attr im 2. byte
			*zp++ = *qp++;
			*zp++ = *qa++;

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

int32 UlaZxsp::doFrameFlyback(int32 /*cc*/) // called from runForSound()
{
	uint int_dur = machine->model == pentagon128 ? 36 // 36:	pentagon128.gif from sblive.narod.ru
												   :
												   48; // 48:	comments on .rzx
	cpu->setInterrupt(0, int_dur);					   // Interrupt

	current_frame++; // flash phase

	if (screen)
	{
		updateScreenUpToCycle(cc_frame_end);	 // screen
		ccx = lines_before_screen * cc_per_line; // update_screen_cc

		record_ioinfo(cc_frame_end, 0xfe, 0); // for 60Hz models: remainder of screen is black
		assert(dynamic_cast<gui::ScreenZxsp*>(screen));
		bool new_buffers_in_use = static_cast<gui::ScreenZxsp*>(screen)->ffb_or_vbi(
			ioinfo, ioinfo_count, attr_pixel, cc_screen_start, cc_per_side_border + 128, getFlashPhase(),
			90000 /*cc_frame_end*/);

		if (new_buffers_in_use)
		{
			std::swap(ioinfo, alt_ioinfo);
			std::swap(ioinfo_size, alt_ioinfo_size);
			std::swap(attr_pixel, alt_attr_pixel);
		}

		ioinfo_count = 0;
		record_ioinfo(0, 0xfe, ula_out_byte);
	}

	return cc_frame_end; // cc_per_frame for last frame
}

void UlaZxsp::drawVideoBeamIndicator(int32 cc) // called from runForSound()
{
	updateScreenUpToCycle(cc);
	assert(dynamic_cast<gui::ScreenZxsp*>(screen));
	bool new_buffers_in_use = static_cast<gui::ScreenZxsp*>(screen)->ffb_or_vbi(
		ioinfo, ioinfo_count, attr_pixel, cc_screen_start, cc_per_side_border + 128, getFlashPhase(), cc);

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
		memcpy(attr_pixel, alt_attr_pixel, bytes_per_octet * n);
	}
}


/*	50 / 60 Hz Umschaltung
	Diese Funktion passt für 48K Spectrums und TK90x/TK95
Chilenischer NTSC Spectrum:
	The CPU is clocked at 3.5275 MHz
	One frame lasts 0xe700 (59136) tstates, giving a frame rate of 3.5275×106 / 59136 = 59.65 Hz
	224 tstates per line implies 264 lines per frame.
	The first contended cycle is at 0x22ff (8959). This implies 40 lines of upper border, 192 lines of picture and 32
lines of lower border/retrace. The contention pattern is confirmed as being the same 6,5,4,3,2,1,0,0 as on the 48K
machine.
*/
void UlaTk90x::set60Hz(bool is60hz)
{
	assert(machine->model == tk90x || machine->model == tk95);
	info = is60hz ? machine->model_info : &zx_info[zxsp_i3];

	lines_before_screen = info->lines_before_screen; // 63 or 64 for 50 Hz
	// lines_in_screen	= m->lines_in_screen;			// 192
	lines_after_screen = info->lines_after_screen;	// 56 for 50 Hz
	cc_per_line		   = info->cpu_cycles_per_line; // Total cpu cycles per line

	Ula::set60Hz(is60hz);
	setupTiming();
}


/*void UlaZxsp::set60Hz(bool is60hz)
{
	assert(machine->model_info->frames_per_second < 55);
	info = is60hz ? &zx_info[tk95] : machine->model_info;

	lines_before_screen	= info->lines_before_screen;	// 63 or 64 for 50 Hz
	//lines_in_screen	= m->lines_in_screen;			// 192
	lines_after_screen	= info->lines_after_screen;		// 56 for 50 Hz
	cc_per_line			= info->cpu_cycles_per_line;	// Total cpu cycles per line

	Ula::set60Hz(is60hz);
	setupTiming();
}*/
