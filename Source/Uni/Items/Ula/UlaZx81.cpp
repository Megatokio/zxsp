// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	ZX81 ULA emulation
	------------------

	--> https://www.sinclairzxworld.com/viewtopic.php?t=560
	--> http://searle.x10host.com/zx80/zx80nmi.html


ZX81 SYNC Generation
====================

((all logic are positive logic, not voltage level of the signals))

Internal State:
	HSYNC counter
	NMI enable flipflop
	VSYNC flipflop
	LCNTR

Outputs:
	pixel data
	combined SYNC
	WAIT signal for CPU
	NMI signal for CPU
	INT signal for CPU

Inputs:
	INTACK = M1 & IORQ
	IN(FE)
	OUT(FF)
	OUT(FE)
	OUT(FD)
	RFSH, r.D6
	HALT


SCREEN BYTE

If a instruction is read from an address with A15=1 then the byte is read from address with A15=0 instead.
If byte.D6=0 then it is executed.
If byte.D6=1 then a NOP is executed instead and the byte is used in the refresh cycle to read a byte
with 8 pixels for the Video output. Unless you cut some wires this byte is always read from the internal ROM.

MASKABLE INTERRUPT

An interrupt is generated when an instruction ends with a RFSH cycle and A6 of the refresh address is 0.
Only instructions which actually end with the RFSH cycle can trigger the interrupt. On the other hand,
r.D6 will be 0 half of the time and so interrupts are normally always disabled in the CPU.

HSYNC PULSE

The HSYNC pulse is generated by the HSYNC counter. It is clocked by cpu clock (at full clock cycles), has a
period of 207cc and activates HSYNC for 16 cycles. The HSYNC pulse cannot be disabled. (and messes up audio output.)

When an interrupt is accepted by the CPU it activates M1 and IORQ simultaneously, which is called the INTACK.
This condition resets the HSYNC counter. The counter is reset immediately when INTACK becomes active and
released when it becomes inactive.
The first HSYNC pulse is 16 cc after release.

NMI and WAIT

NMI is enabled by OUT(FE) and disabled by OUT(FD).

If NMI is enabled, then NMI is active at the same time as the HSYNC pulse.
IF NMI is raised early enough during an OUT(FD) instruction then the NMI is still accepted after the instruction.
It must be active 1cc before the end of the instruction. but it is in a race condition with the HSYNC output which
toggles at full cc. The HSYNC counter output runs through some gates which combine into the HSYNC signal, so it
probably toggles after /NMI is tested by the CPU. Therefore the HSYNC must go active at instruction_end-2cc to be seen.

If NMI is active then WAIT is active at the same time, except if HALT is true during HALT opcode repetitions.
This applies to opcode fetch, memory read & write, input, output, NMIACK and INTACK (IO cycle and pushes).
The waitmap scheme is normal, not like the Sinclair ZX Spectrum with many superfluous wait cycles e.g. in 'jr'.
WAIT is tested at cc+1.5 in memory and cc+2.5 in io access. Therefore it must be asserted at cc+1 or cc+2 respectively.
WAIT is tested at cc+3.5 in INT acknowledge cycle.
WAIT is tested at cc+1.5 in NMI acknowledge cycle. (as in any opcode fetch cycle)

VSYNC PULSE

If NMI is disabled then a VSYNC pulse is started by IN(FE).
The VSYNC pulse is ended by any OUT(FF).
The combined SYNC output of the ZX81 is the logical OR of HSYNC and VSYNC.

LCNTR

A 3 bit counter is used to count the lines within each row of characters. It is part of the address created
during RFSH cycle to read the byte for the video output.

The LCNTR is reset while the VSYNC pulse is active and it is clocked by the leading edge of the HSYNC pulse.

ATTN: There may be more going on here because some highres games (Hangman_ARX) displays everything rolled up
by 1 line inside character cells. But this may also stem from somewhere else. to be investigated.


WAIT test and related timing
============================

	The Z80 tests WAIT 1.5cc after start of opcode read cycle.
	ZXSP uses 2cc as offset.
	If WAIT is asserted at +1cc then it is seen by the Z80 cpu.
	If WAIT is asserted at +2cc then it is seen by the Z80 cpu.
	Therefore a test at +1cc would have been better.
	But that is now hard to fix because we have to update a lot of places.
	Therefore we also add +1cc to every timing point:

	WAIT test at:

	in INSTR	+1.5	eff. +1	  zxsp +2
	in RD,WR	+1.5	eff. +1	  zxsp +2
	in IO   	+2.5	eff. +2	  zxsp +3
	in NMI		+1.5 	eff. +1	  zxsp +2
	in INT  	+3.5 	eff. +3	  zxsp +4

	NMI and INT test at:

	endof INSTR	-1.0	eff. -2	  zxsp -1		effectively 1cc later because of race condition:
												HSYNC toggles at full cc and NMI is tested at full cc

	HSYNC counter reset at:

	INTACK	 	+2.5	eff. +2	  zxsp +3		(?) (important?)
		to		+4.0	eff. +4	  zxsp +5
*/

#include "UlaZx81.h"
#include "Dsp.h"
#include "Interfaces/IScreen.h"
#include "Keyboard.h"
#include "Machine.h"
#include "TapeRecorder.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"
#include <functional>


#define o_addr "----.----.----.----" // übliche Adresse: $FF
#define i_addr "----.----.----.---0" // übliche Adresse: $FE

// bits 0-4: 5 keys from keyboard (low=pressed), row selected by A8..A15
static constexpr uint8 FRAMERATE_MASK = 1 << 6; // 50/60 Hz Wahlschalter (Lötbrücke)
static constexpr uint8 EAR_IN_MASK	  = 1 << 7; // audio input


// WAITMAP_POS should be +1 because of the test position in INSTR, MEMRQ and IORQ in Z80macros.h
// NMI_POS is the number of cc before instr end / nmi handling start
//			if NMI_POS is too early then more than the max. possible 13 wait cycles are encountered.
//			if NMI_POS is too late then 13 wait cycles are never encountered.
static constexpr int32 WAITMAP_POS = +1; // +1 wg. test position in Z80.run()
static constexpr int32 NMI_POS	   = +2; // +2 = waitmap_pos+1 => max. 13 wait cycles


UlaZx81::~UlaZx81() {}

UlaZx81::UlaZx81(Machine* m) : UlaZx80(m, isa_UlaZx81, o_addr, i_addr)
{
	m->cpu_options |= cpu_waitmap; // no cpu_ula_sinclair
}

void UlaZx81::powerOn(int32 cc)
{
	xlogIn("UlaZx81:powerOn");
	UlaZx80::powerOn(cc);

	// note: in a real ZX81 the power-up states of ULA registers may be undetermined!
	clear_waitmap();
	lcntr		  = 0;	   // 3 bit scanline counter
	nmi_enabled	  = false; // zx81: no default state!
	vsync		  = off;
	hsync		  = off;
	sync		  = off;
	cc_hsync_next = cc + 16;
}


// -------------------------------------------------------------
//					HSYNC, VSYNC and NMI timing
// -------------------------------------------------------------

inline void UlaZx81::clear_waitmap() { memset(waitmap, 0, waitmap_size); }

void UlaZx81::setup_waitmap(int32 cc_hsync)
{
	// when NMI is enabled then /NMI and /WAIT are asserted for 16cc at the same time as HSYNC.
	// the Z80 emulation will use cc % 207 to peek into the waitmap[].
	// the NMI will start at cc_nmi => waitmap[cc_nmi%207 to cc_nmi%207+15] must be set.

	// the waitmap must be updated
	// - when the NMI is enabled
	// - when the cc timebase is shifted in videoFrameEnd()
	// - when the HSYNC generator is restarted in interruptAtCycle()

	cc_hsync += waitmap_size;
	assert(cc_hsync >= 0);
	cc_hsync %= waitmap_size;

	if (waitmap[cc_hsync] == 16) // quick test whether it is already set as required
		return;
	clear_waitmap(); // clear old wait positions

	// circularly fill in 16cc delay for the first cc of the NMI pulse to 1cc for the last:
	int i = 0;
	for (; i < min(int(waitmap_size) - cc_hsync, 16); i++) { waitmap[cc_hsync + i] = uint8(16 - i); }
	for (; i < 16; i++) { waitmap[cc_hsync - int(waitmap_size) + i] = uint8(16 - i); }
}

void UlaZx81::set_sync(int32 cc, bool f)
{
	// set or clear the combined sync output
	// this propagates the new sync state to the tv decoder
	// and tries to detect the end of a frame flyback (vsync).

	if (f == sync) return;

	sync = f;
	tv_decoder.syncOn(cc, f);
}

void UlaZx81::run_hsync(int32 cc)
{
	// run the HSYNC counter up to clock cycle cc

	while (cc_hsync_next <= cc)
	{
		hsync = !hsync;

		if (vsync == off) set_sync(cc_hsync_next, hsync);

		if (vsync == off) // else vsync => LCNTR.reset
			if (hsync == on) lcntr = (lcntr + 1) & 7;

		cc_hsync_next += hsync ? 16 : cc_hsync_period - 16;
	}
}

inline void UlaZx81::reset_hsync(int32 cc, int32 dur)
{
	// reset the HSYNC counter.
	// the reset input is activated by an INT acknowledge cycle of the cpu.
	// the HSYNC counter is kept in reset while the INTACK signal is active.
	// => the alignment of the NMI changes => the waitmap needs to be updated

	assert(cc < cc_hsync_next); // run() must have been called

	hsync = off;
	set_sync(cc, vsync);

	cc_hsync_next = cc + dur + 16;

	if (nmi_enabled)
	{
		setup_waitmap(cc_hsync_next + WAITMAP_POS);
		if (machine->cpu->nmiCycle() - NMI_POS >= cc) // if not yet triggered
			machine->cpu->setNmi(cc_hsync_next + NMI_POS);
	}
}

inline void UlaZx81::clear_vsync(int32 cc)
{
	// reset the VSYNC flipflop.
	// the reset input is activated by any OUTPUT cycle of the cpu.

	assert(cc < cc_hsync_next); // hsync_run() must have been called

	vsync = off;
	set_sync(cc, hsync);
}

inline void UlaZx81::set_vsync(int32 cc)
{
	// set the VSYNC flipflop.
	// the set input is activated by INPUT(0xFE) if vsync enabled (== nmi disabled).
	// lcntr is kept in reset while the vsync ff is set.

	assert(cc < cc_hsync_next); // run_hsync() must have been called
	assert(vsync_enabled());

	vsync = on;
	set_sync(cc, on);
	lcntr = 0;
}

inline void UlaZx81::disable_nmi(int32 cc)
{
	// reset the NMI_enable flipflop.
	// the reset input is activated by OUTPUT(0xFD)

	assert(cc < cc_hsync_next); // run() must have been called
	assert(nmi_enabled);

	nmi_enabled = false;
	clear_waitmap();

	if (machine->cpu->nmiCycle() - NMI_POS >= cc) machine->cpu->clearNmi();
}

inline void UlaZx81::enable_nmi(int32 cc)
{
	// set the NMI_enable flipflop.
	// the set input is activated by OUTPUT(0xFE)

	assert(cc < cc_hsync_next); // run() must have been called
	assert(nmi_enabled == false);

	nmi_enabled = true;
	machine->cpu->setNmi(hsync ? cc + NMI_POS : cc_hsync_next + NMI_POS);

	int32 cc_hsync = hsync ? cc_hsync_next - 16 + cc_hsync_period : cc_hsync_next;
	setup_waitmap(cc_hsync + WAITMAP_POS);
}

void UlaZx81::output(Time now, int32 cc, uint16 addr, uint8)
{
	// handle any OUT(…) instruction
	// cc = start of machine cycle
	cc -= 3;

	// on a real ZX81 the audio_out signal is picked from the combined sync signal.
	// this results in a permanent hissing in the tape recording.
	// since this is easier to do and unwanted anyway we pick it from the VSYNC only:
	mic_out(now, cc, 0);

	run_hsync(cc + 1);
	clear_vsync(cc + 1); // vsync off immediately (unlike ZX80)

	if ((addr & 3) == 2) // A0=0 & A1=1: enable nmi
	{
		if (!nmi_enabled) enable_nmi(cc + 1);
	}

	if ((addr & 3) == 1) // A1=0 & A0=1: disable nmi
	{
		if (nmi_enabled) disable_nmi(cc + 1);
	}

	if (nmi_enabled) // add waitstates:
	{
		int d = waitmap[uint(cc + 2 + WAITMAP_POS) % waitmap_size];
		machine->cpu->cpuCycleRef() += d;
	}
}

void UlaZx81::input(Time now, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	// handle IN(0xFE) instruction
	// cc = start of machine cycle
	cc -= 3;

	// on a real ZX81 the audio_out signal is picked from the combined sync signal.
	// this results in a permanent hissing in the tape recording.
	// since this is easier to do and unwanted anyway we pick it from the VSYNC only:
	if (vsync_enabled()) mic_out(now, cc, 1);

	run_hsync(cc + 1);
	if (vsync_enabled()) // set the VSYNC flipflop if VSYNC enabled (== NMI disabled)
		set_vsync(cc + 1);

	if (nmi_enabled) // add waitstates
	{
		int d = waitmap[uint(cc + 2 + WAITMAP_POS) % waitmap_size];
		machine->cpu->cpuCycleRef() += d;
		cc += d;
	}

	mask = 0xff; // handling of floating bus byte is only required for ZXSP.

	// insert bits from keyboard:
	byte &= readKeyboard(addr);

	// insert bit D6 from framerate jumper:
	if (is60hz) byte &= ~FRAMERATE_MASK;

	// insert bit D7 from EAR input socket:
	if (!mic_in(now, cc)) byte &= ~EAR_IN_MASK;
}

void UlaZx81::crtcRead(int32 cc, uint opcode)
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
	// in case of the ZX80 4k ROM A12 must be 0. (handled by memory mapping in MmuZX80)
	// A13 is probably ignored. (handled by memory mapping in MmuZX80)

	run_hsync(cc + 4);
	if (sync == on) return; // SYNC => BLACK!

	Z80*  cpu = machine->cpu;
	uint  ir  = cpu->getRegisters().ir; // i register
	uchar b	  = cpu->peek(uint16((ir & 0x3e00) | ((opcode << 3) & 0x01f8) | lcntr));
	tv_decoder.storePixelByte(cc + 4, opcode & 0x0080 ? b : ~b);
}

int32 UlaZx81::nmiAtCycle(int32 cc)
{
	// the cpu started a NMIACK cycle at clock cycle cc.
	// this has no side effects in the SYNC generation
	// but it needs to be re-scheduled.

	assert(nmi_enabled);
	run_hsync(cc);

	int32 cc_nmi = hsync ? cc_hsync_next - 16 + cc_hsync_period + NMI_POS : cc_hsync_next + NMI_POS;
	return cc_nmi; // re-shedule nmi
}

uint8 UlaZx81::interruptAtCycle(int32 cc, uint16 /*pc*/)
{
	// the cpu started an INTACK cycle at clock cycle cc
	// the INTACK (/IORQ & /M1) signal resets the counter of the HSYNC generator
	// /IORQ goes low at cc + 2.5 and high again at cc + 4
	// for the duration of INTACK the HSYNC counter is held in reset.

	assert(machine->cpu->interruptStart() == cc - 2); // expected start in prev. resfresh cycle

	run_hsync(cc + 2);		// HSYNC.reset activated after cc+2
	reset_hsync(cc + 2, 2); // HSYNC.reset active until cc+4

	return 0xff; // byte read during INT ACK cycle
}

int32 UlaZx81::updateScreenUpToCycle(int32 cc)
{
	run_hsync(cc);
	return 1 << 30; // ZXSP only
}

void UlaZx81::videoFrameEnd(int32 cc)
{
	// shift the cpu cycle based time base by cc:
	// this should be the time for the previous frame.

	assert(cc > 0);
	cc_hsync_next -= cc;

	if (nmi_enabled)
	{
		int32 cc_hsync = hsync ? cc_hsync_next + cc_hsync_period - 16 : cc_hsync_next;
		setup_waitmap(cc_hsync + WAITMAP_POS);
	}

	tv_decoder.shiftCcTimeBase(cc);
}

int32 UlaZx81::doFrameFlyback(int32 cc)
{
	// handle vertical frame flyback
	// and return actual duration of last frame.
	// this will be the offset used to shift cc in videoFrameEnd()
	// which should reset cc_frame_start back to 0.

	run_hsync(cc);
	return UlaZx80::doFrameFlyback(cc);
}

void UlaZx81::drawVideoBeamIndicator(int32 cc)
{
	run_hsync(cc);
	tv_decoder.drawVideoBeamIndicator(cc);
}
