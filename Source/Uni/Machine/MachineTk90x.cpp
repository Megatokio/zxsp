// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTk90x.h"
#include "Joy/SinclairJoy.h"
#include "Keyboard.h"
#include "MachineZxsp.h"
#include "TapeRecorder.h"


MachineTk90x::MachineTk90x(gui::MachineController* m, bool is60hz) : MachineZxsp(m, tk90x, isa_MachineTk90x)
{
	cpu		 = new Z80(this);
	ula		 = new UlaTk90x(this, is60hz);
	mmu		 = new MmuZxsp(this);
	keyboard = new KeyboardZxsp(this, isa_KbdTk90x);
	// ay		=
	joystick = new Tk90xJoy(this);
	// fdc		=
	// printer	=
	taperecorder = new TS2020(this);
}


/*	TODO

Reading from port $FE produces different results in Brazilian computers. In a nutshell:

	Input port $FE bit 7: always value 0 in TK90X, always value 1 in TK95 and ZX-Spectrum.
	Input port $FE bit 6: default value 1 (when there's no input signal) in both TK90X and TK95.
	Input port $FE bit 5: usually 1 in TK90X, but when ULA accesses a screen attribute address,
						  it will copy bit 5 from the attribute value.

	The information above was extracted from an article by Flavio Matsumoto,
	based on additional information identified by Fabio Belavenuto.
	The complete article (in Portuguese) is available here:

	http://cantinhotk90x.blogspot.com.br...porta-254.html


wikipedia:

	the TV system was hardware selectable to PAL-M (60 Hz) as used in Brazil,
		PAL-N (50 Hz) as used in Uruguay, Argentina and Paraguay
		and NTSC (60 Hz) as used in USA and many other countries.

wikia.com:
	About a NTSC Specrum, but timings may be similar:
	One example of an NTSC Spectrum (as opposed to a Timex machine) has been found (in Chile).
	As far as is known, it is the same as a normal (PAL) Spectrum with the following differences:
		The CPU is clocked at 3.5275 MHz
		The ULA is a model 6C011E-3 which generates a NTSC frame size and rate
		One frame lasts 0xe700 (59136) tstates, giving a frame rate of 3.5275×106 / 59136 = 59.65 Hz
		224 tstates per line implies 264 lines per frame.
		The first contended cycle is at 0x22ff (8959).
		This implies 40 lines of upper border, 192 lines of picture and 32 lines of lower border/retrace.
		The contention pattern is confirmed as being the same 6,5,4,3,2,1,0,0 as on the 48K machine.

	Many thanks to Claudio Bernet for running all the tests on his machine.
*/
