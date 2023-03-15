// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/* 	ZX80 ULA emulation
	------------------

	Input address:	0xFE  -  A0=0: activate VSYNC - any other IN create spurious HSYNC pulses!
	INTACK:         create HSYNC pulse
	Output address:	0xFF  -  any address: deactivate VSYNC - any OUT will do it!

	The MIC_OUT output is directly taken from the SYNC signal.

	IN A,(FE)	activate the SYNC-Signal (normally for VSYNC)
				resets the 3-bit line counter LCNTR.
				reads 5 bits from the keyboard matrix (high byte selects row)
				reads 50/60 Hz frame rate jumper
				reads EAR_IN input

	OUT (FF),A	deactivates the SYNC signal

	INT			Reading an opcode when refresh register R.D6=0 triggers an interrupt immediately after this M1 cycle.

	INTACK		activates the SYNC signal after 17cc for 20cc. (standard ZX80 ROM INT handler)
				this depends on the timing of the following 4 M1 cycles (1st of which is the INTACK M1 cycle)

	A15=1 D6=0	Reading an opcode at address A15=1 actually reads from address with A15=0
				and returns a NOP if D6=0.
				Then the byte is used together with LCNTR and I register to form the address of a
				pixel byte which is read in the refresh cycle of the same M1 cycle from the the ROM
				and stored into the video shifter.
				Every 4 cpu clock cycles one byte must be read to produce a scanline for the video image.
*/

#include "UlaZx80.h"
#include "Interfaces/IScreen.h"
#include "Keyboard.h"
#include "Machine.h"
#include "TapeRecorder.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"


#define o_addr "----.----.----.----" // übliche Adresse: $FF
#define i_addr "----.----.----.----" // übliche Adresse: $FE, but we need to see all IN as well

// bits 0-4: 5 keys from keyboard (low=pressed), row selected by A8..A15
static constexpr uint8 FRAMERATE_MASK = 1 << 6; // 50/60 Hz Wahlschalter (Lötbrücke)
static constexpr uint8 EAR_IN_MASK	  = 1 << 7; // audio input


UlaZx80::~UlaZx80() {}

UlaZx80::UlaZx80(Machine* m, isa_id _id, cstr oaddr, cstr iaddr) :
	Ula(m, _id, oaddr, iaddr),
	tv_decoder(screen, int32(m->model_info->cpu_cycles_per_second))
{}

UlaZx80::UlaZx80(Machine* m, bool is60hz) : UlaZx80(m, isa_UlaZx80, o_addr, i_addr) { UlaZx80::set60Hz(is60hz); }

void UlaZx80::powerOn(int32 cc)
{
	xlogIn("UlaZx80:powerOn");
	Ula::powerOn(cc);

	assert(screen);
	tv_decoder.reset(screen);
	set60Hz(is60hz);
	beeper_volume		  = 0.0025f;
	beeper_current_sample = -beeper_volume;
	lcntr				  = 0; // 3 bit scanline counter
	vsync				  = off;
}

void UlaZx80::reset(Time t, int32 cc)
{
	xlogIn("UlaZx80::reset");
	Ula::reset(t, cc);
}

void UlaZx80::set60Hz(bool is_60hz)
{
	bool machine_is60hz = machine->model_info->frames_per_second > 55;
	info				= machine_is60hz == is_60hz ? machine->model_info : &zx_info[is_60hz ? ts1000 : zx80];

	// these will be updated while running anyway:
	cc_per_line			= int(info->cpu_cycles_per_line);
	lines_before_screen = int(info->lines_before_screen);
	lines_in_screen		= int(info->lines_in_screen);
	lines_after_screen	= int(info->lines_after_screen);
	lines_per_frame		= lines_before_screen + lines_in_screen + lines_after_screen;

	Ula::set60Hz(is_60hz);
}

void UlaZx80::enableMicOut(bool f)
{
	beeper_volume		  = f ? 0.05f : 0.0025f;
	beeper_current_sample = beeper_current_sample > 0.0f ? beeper_volume : -beeper_volume;
}

void UlaZx80::mic_out(Time now, int32 cc, bool bit)
{
	machine->outputSamples(beeper_current_sample, beeper_last_sample_time, now);
	beeper_last_sample_time = now;
	beeper_current_sample	= bit ? beeper_volume : -beeper_volume;

	if (machine->taperecorder->isRecording()) machine->taperecorder->output(cc, bit);
}

bool UlaZx80::mic_in(Time now, int32 cc)
{
	constexpr Sample threshold = 0.01f; // who cares

	if (machine->taperecorder->isPlaying()) { return machine->taperecorder->input(cc); }

	if (machine->audio_in_enabled)
	{
		uint32 a = uint32(now * samples_per_second);
		if (unlikely(a >= DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING))
		{
			assert(int32(a) >= 0);
			showAlert("Sample input beyond dsp buffer: +%i\n", int(a - DSP_SAMPLES_PER_BUFFER));
		}
		else { return machine->audio_in_buffer[a] >= threshold; }
	}

	return 0 >= threshold;
}

void UlaZx80::output(Time now, int32 cc, uint16 /*addr*/, uint8 /*byte*/)
{
	if (vsync)
	{
		// any out() deactivates the VSYNC signal after this M1 cycle
		// output() is called at cc = 3 in the i/o cycle => cc+1 for the end
		tv_decoder.syncOff(cc + 1);
		vsync = false;
		assert(lcntr == 0);
		mic_out(now, cc, 1);
	}
}

void UlaZx80::input(Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	if (addr & 0x0001)
	{
		// A0=1: not my address
		// this set's FF1 which after 2 M1 cycles will activate SYNC
		// and after 2 more M1 cycles deactivate SYNC again
		// just like in INT request handling, only with even less knowledge about the future timing.

		// as this is too high frequency for audio we assume that is not intended for audio and ignore it.
		// else we'd need to assert that we don't step back in time which may kill Dsp::outputSamples()

		if (vsync == false)
		{
			// we'll use the same timing as for INT
			// we arbitrarily start counting at the estimated start of the IN opcode at cc-3-4
			tv_decoder.syncOn(cc - 3 - 4 + 17);
			tv_decoder.syncOff(cc - 3 - 4 + 17 + 20);
			lcntr = (lcntr + 1) & 7;
		}
		return;
	}

	// A0=0:

	if (vsync == false)
	{
		// any in(FE) activates the VSYNC signal and resets the LCNTR of the ULA

		// input() is called at cc = +3 in the i/o cycle
		// but /IORQ was activated at cc+1  =>  subtract 2
		tv_decoder.syncOn(cc - 2);
		vsync = true;
		lcntr = 0;
		mic_out(now, cc, 0);
	}

	mask = 0xff; // though D5 is not driven. handling of floatingbus byte is only required for ZXSP.

	// insert bits from keyboard:
	byte &= readKeyboard(addr);

	// insert bit D6 from framerate jumper:
	if (is60hz) byte &= ~FRAMERATE_MASK;

	// insert bit D7 from EAR input socket:
	if (!mic_in(now, cc)) byte &= ~EAR_IN_MASK;
}

void UlaZx80::crtcRead(int32 cc, uint opcode)
{
	// an instruction was read at an address with A15=1 and returned an opcode with A6=0
	// => the Ula reads a video byte and fakes a NOP for the CPU
	//	  the NOP is already handled in the Z80 macro

	// bits D0 … D5 of the opcode are used as character code.
	// D7 is used to complement the output.

	// the screen byte is read from an address composed as following:
	// A0…2 = LCNTR
	// A3…A8 = D0…D5 from opcode
	// A9…A15 = A1…A7 from I register

	// the byte is always read from the internal ROM. (always activated)
	// but in case of the 4k ROM A12 must be 0. (handled by memory mapping in MmuZX80)
	// A13 is probably ignored. (handled by memory mapping in MmuZX80)

	if (unlikely(vsync)) return; // SYNC => BLACK!

	Z80*  cpu = machine->cpu;
	uint  ir  = cpu->getRegisters().ir; // i register
	uchar b	  = cpu->peek(uint16((ir & 0x3e00) | ((opcode << 3) & 0x01f8) | lcntr));
	tv_decoder.storePixelByte(cc + 4, opcode & 0x0080 ? b : ~b);
}

uint8 UlaZx80::interruptAtCycle(int32 cc, uint16 /*pc*/)
{
	//	special callback for ZX80/ZX81 screen update:

	/*
	Interrupt acknowledge:
	=> IORQ, WR and M1 are active at the same time
	=> the SYNC signal is activated after 2 M1 cycles
	   and deactivated after 2 more M1 cycles (~6.15µs)

	cc is the cycle of start of interrupt handling.

	The INT signal was generated when D6=0 during refresh cycle in previous opcode
	and it was sampled by the cpu during this time.
	The 3-bit shift register is clocked by the M1 signal and
	therefore the VSYNC signal toggles at the start of an M1 cycle (~ start of opcode)

	The next 4 M1 cycles are:
		13cc  INT ACK cycle of the cpu
		4cc   DEC C			; INT handler at 0x0038 in ZX80 rom
		10cc  JP NZ,L0045	;
		10cc  POP HL  or  POP DE if jumped
	*/

	assert(machine->cpu->interruptStart() == cc - 2);

	if (vsync == false)
	{
		const int cc_sync_on  = cc + 13 + 4;
		const int cc_sync_off = cc + 13 + 4 + 20;

		tv_decoder.syncOn(cc_sync_on);
		tv_decoder.syncOff(cc_sync_off);
		lcntr = (lcntr + 1) & 7;
	}

	return 0xff; // byte read during INT ACK cycle
}

int32 UlaZx80::updateScreenUpToCycle(int32)
{
	return 1 << 30; // ZXSP only
}

int32 UlaZx80::cpuCycleOfFrameFlyback()
{
	// return the estimated time for the end of the current/next frame.
	// the machine will run up to this cc and probably overshoot by some cc.

	return tv_decoder.getCcForFrameEnd();
}

void UlaZx80::videoFrameEnd(int32 cc)
{
	// shift the cpu cycle based time base by cc:
	// this should be the time for the previous frame.

	tv_decoder.shiftCcTimeBase(cc);
}

int32 UlaZx80::doFrameFlyback(int32 cc)
{
	// handle vertical frame flyback
	// and return actual duration of last frame.
	// this will be the offset used to shift cc in videoFrameEnd()
	// which should reset cc_frame_start back to 0.

	cc = tv_decoder.doFrameFlyback(cc);

	lines_before_screen = tv_decoder.lines_above_screen;
	lines_in_screen		= tv_decoder.lines_in_screen;
	lines_after_screen	= tv_decoder.lines_below_screen;
	lines_per_frame		= tv_decoder.lines_per_frame;
	cc_per_line			= tv_decoder.getCcPerLine();

	return cc;
}

void UlaZx80::drawVideoBeamIndicator(int32 cc) { tv_decoder.drawVideoBeamIndicator(cc); }
