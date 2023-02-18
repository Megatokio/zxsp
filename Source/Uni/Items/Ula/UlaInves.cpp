// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaInves.h"
#include "Dsp.h"
#include "Machine.h"
#include "Qt/Screen/Screen.h"
#include "TapeRecorder.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"


#define MIN_LINES_BEFORE_SCREEN 24
#define MAX_LINES_BEFORE_SCREEN 72

#define MIN_LINES_AFTER_SCREEN 24
#define MAX_LINES_AFTER_SCREEN 2000 // note: lines after screen are used as padding for cpu clock overdrive!

#define MIN_BYTES_PER_LINE (4 + 32 + 4)
#define MAX_BYTES_PER_LINE (16 + 32 + 16)


#define io_addr "----.----.----.---0" // übliche Adresse: $FE

#define KEYBOARD_IN_MASK 0x1F
#define EAR_IN_MASK		 0x40

#define BORDER_OUT_MASK 0x07
#define MIC_OUT_MASK	0x08
#define EAR_OUT_MASK	0x10
#define MIC_OUT_SHIFT	3 // 0x01<<3 = 0x08


#define CC_PER_BYTE 4 // ula cycles per pixel block  ((2 bytes == 8 pixel))


UlaInves::~UlaInves() { xlogIn("~UlaInves"); }


UlaInves::UlaInves(Machine* m) : UlaZxsp(m, isa_UlaInves, io_addr, io_addr)
{
	xlogIn("new UlaInves");
	assert(waitmap_size == 0);
	m->cpu_options &= ~cpu_floating_bus; // undo set in UlaZxsp ctor
}


/*	setup timing parameters
	in:	cc_per_line
		lines_before_screen
		lines_in_screen
		lines_after_screen
*/
void UlaInves::setupTiming()
{
	// validate settings.:
	int bytes_per_line = cc_per_line / CC_PER_BYTE;
	limit(MIN_BYTES_PER_LINE, bytes_per_line, MAX_BYTES_PER_LINE);
	cc_per_line = bytes_per_line * CC_PER_BYTE;

	limit(MIN_LINES_BEFORE_SCREEN, lines_before_screen, MAX_LINES_BEFORE_SCREEN);
	limit(MIN_LINES_AFTER_SCREEN, lines_after_screen, MAX_LINES_AFTER_SCREEN);
	// lines_in_screen = 192;
	lines_per_frame = lines_before_screen + lines_in_screen + lines_after_screen;

	// Abgeleitete Timing-Werte
	cc_per_side_border = cc_per_line - 32 * CC_PER_BYTE;
	cc_frame_end	   = lines_per_frame * cc_per_line;

	// von wann bis wann soll die cpu die waitmap benutzen?
	cc_waitmap_start = cc_frame_end; // never
	cc_waitmap_end	 = cc_frame_end;

	// startcycle für den CRTC
	cc_screen_start = lines_before_screen * cc_per_line;

	// ccx validieren:
	{
		int row = ccx / cc_per_line - lines_before_screen;
		int col = ccx % cc_per_line / CC_PER_BYTE / 2 * 2; // in screen: 0*2 .. 15*2; else in side boder

		if (row < 0) { row = col = 0; }
		if (col > 30)
		{
			col = 0;
			row += 1;
		}
		if (row >= lines_in_screen)
			ccx = 1 << 30;
		else
			ccx = (lines_before_screen + row) * cc_per_line + col * CC_PER_BYTE;
	}

	// ccb validieren:
	//	ccb = ccb;		// ccb ist der byte-index in border[]

	// Info
	xlogline("UlaInves:setup_timing:");
	xlogline("+ lines_before_screen = %i", lines_before_screen);
	xlogline("  lines_in_screen     = %i", lines_in_screen);
	xlogline("  lines_after_screen  = %i", lines_after_screen);

	xlogline("+ cc_per_line         = %i", cc_per_line);
	xlogline("  cc_per_side_border  = %i", cc_per_side_border);

	xlogline("+ cc predivider       = %i", 5);
	xlogline("  cc_per_line         = %i", cc_per_line);
	xlogline("  cc_screen_start     = %i", cc_screen_start);
	xlogline("  cc_frame_end        = %i", cc_frame_end);
}


/* ---------------------------------------------------------------
				Input from ULA
--------------------------------------------------------------- */

void UlaInves::input(Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	assert(~addr & 1); // not me
	// the Inves Ula has 5 uc/cc so i assume that ula and cpu memory access are properly interleaved
	// and the cpu cannot sneak ula bytes from the bus

	// input from Ula:

	// bits 0-4 come from the keyboard	(handled by item Keyboard)
	// bits 5+7 are always 1
	// bit  6	is the signal from the ear socket messed up with ear+mic outputs
	mask = 0xff; // to be tested

	byte &= readKeyboard(addr); // Ähh.. TODO: wird auch high getrieben?

	if (machine->taperecorder->isPlaying())
	{
		if (!machine->taperecorder->input(cc)) byte &= ~EAR_IN_MASK;
	}
	else if (machine->audio_in_enabled)
	{
		// insert bit from mic socket
		if (ula_out_byte & EAR_OUT_MASK) // EAR_OUT==1
		{
			// ULA_EAR_IN is **probably** always "1"  =>  nothing to do
			// verified 2006-06-29 for issue 2, 4A, and 6U (128K)
		}
		else // EAR_OUT==0
		{
			// signal from EAR input is read into bit 6.
			// threshold level depends on state of EAR output bit and PCB issue/model!
			// TODO: simulate capacitor

			Sample threshold = (ula_out_byte >> MIC_OUT_SHIFT) & 1 ? machine->model_info->earin_threshold_mic_lo :
																	 machine->model_info->earin_threshold_mic_hi;

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
	else
		byte &= ~EAR_IN_MASK;
}


int32 UlaInves::addWaitCycles(int32 cc, uint16 /*addr*/) const volatile { return cc; }

uint8 UlaInves::getFloatingBusByte(int32 /*cc*/) { return 0xff; }
