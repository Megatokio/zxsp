// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	http://www.worldofspectrum.org/faq/reference/128kreference.htm;

	Bit 6 of Port 0xfe of the +2A/+3 does not show the same dependence on what was written to Port 0xfe
	as it does on the other machines, and always returns 0 if there is no signal.
	Reading from a non-existing port (eg 0xff) will always return 255,
	and not give any screen/attribute bytes as it does on the 48K/128K/+2.

	Unlike the base 128K machine, RAM banks 4, 5, 6 and 7 are contended.
	However, Port 0xfe is not; whether ports 0x7ffd and 0x1ffd are contended is currently unknown.

	The other difference occurs for instructions which have multiple 'pc+1' or 'hl' entries
	in the breakdown for the other machines: on the +2A/+3, these entries are combined into just one.
	This means that, for example, JR dis – formerly pc:4,pc+1:3,5*1 – becomes pc:4,pc+1:8.
*/


#include "UlaPlus3.h"
#include "Machine/Machine.h"
#include "TapeRecorder.h"


// #define	MIN_LINES_BEFORE_SCREEN		24
// #define	MAX_LINES_BEFORE_SCREEN		72		// nominal: 63/64
// #define	MAX_LINES_BEFORE_SCREEN_PENTAGON 88	// nominal: 64+16 = 80

// #define	MIN_LINES_AFTER_SCREEN		24
// #define	MAX_LINES_AFTER_SCREEN		2000	// note: lines after screen are used as padding for cpu clock overdrive!

// #define	MIN_BYTES_PER_LINE			( 4+32+4 )
// #define	MAX_BYTES_PER_LINE			(16+32+16)


// io_addr_zxsp = "----.----.----.---0"		übliche Adresse: $FE		BESTÄTIGT
// o_addr_plus3 = "01--.----.----.--0-"		üblicher Port: 0x7ffd		WoS
//											Reading from 0x7ffd returns floating bus values. WoS


#define i_addr "----.----.----.---0"
#define o_addr "----.----.----.----"


// #define KEYBOARD_IN_MASK	0x1F
#define EAR_IN_MASK 0x40

#define BORDER_OUT_MASK 0x07
#define MIC_OUT_MASK	0x08
#define EAR_OUT_MASK	0x10


UlaPlus3::UlaPlus3(Machine* m) : Ula128k(m, isa_UlaPlus3, o_addr, i_addr)
{
	m->cpu_options &= ~cpu_floating_bus; // undo set in UlaZxsp ctor
}


/* ---------------------------------------------------------------
				Output to ULA
	access to +3 Ula is not contended
--------------------------------------------------------------- */

void UlaPlus3::output(Time t, int32 cc, uint16 addr, uint8 byte)
{
	if (~addr & 1) { UlaZxsp::output(t, cc, addr, byte); }
	else
	{
		// test for video page change:
		// o_addr_plus3 = "01--.----.----.--0-"	// üblicher Port: 0x7ffd

		if ((addr & 0xC002) != 0x4000) return; // not the MMU port
		if (mmu_is_locked()) return;		   // mmu port disabled
		uint x	  = byte ^ port_7ffd;
		port_7ffd = byte;
		if (x & 0x08) // video page changed?
		{
			if (cc >= ccx) updateScreenUpToCycle(cc);
			markVideoRam();
		}
	}
}


/* ---------------------------------------------------------------
				Input from ULA
	access to +3 Ula is not contended
	ear_in value does not depend on ear_out value as on SINCLAIR models
--------------------------------------------------------------- */

void UlaPlus3::input(Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	assert(~addr & 1); // Bit A0 = 1  => ULA not selected

	// bits 0-4 come from the keyboard
	// bits 5+7 are always 1
	// bit  6	is the signal from the ear socket
	mask = 0xff; // to be tested

	byte &= readKeyboard(addr);

	const Sample threshold = 0.005f;

	if (machine->taperecorder->isPlaying())
	{
		if (!machine->taperecorder->input(cc)) byte &= ~EAR_IN_MASK;
	}
	else if (machine->audio_in_enabled)
	{
		uint32 a = uint32(now * samples_per_second);
		if (a >= DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING)
		{
			assert(int32(a) >= 0);
			showAlert("Sample input beyond dsp buffer: +%i\n", int(a - DSP_SAMPLES_PER_BUFFER));
			if (0.0f < threshold) byte &= ~EAR_IN_MASK;
		}
		else
		{
			if (machine->audio_in_buffer[a] < threshold) byte &= ~EAR_IN_MASK;
		}
	}
	else if (0.0f < threshold) byte &= ~EAR_IN_MASK;
}


int32 UlaPlus3::addWaitCycles(int32 cc, uint16 /*addr*/) const volatile { return cc; }

uint8 UlaPlus3::getFloatingBusByte(int32 /*cc*/) { return 0xff; }
