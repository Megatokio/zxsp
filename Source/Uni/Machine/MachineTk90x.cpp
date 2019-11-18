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


#include "MachineTk90x.h"
#include "MachineZxsp.h"
#include "Settings.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Joy/SinclairJoy.h"


MachineTk90x::MachineTk90x(MachineController*m)
:
	MachineZxsp(m,tk90x,isa_MachineTk90x)
{
	cpu			= new Z80(this);
	ula			= new UlaTk90x(this); ula->set60Hz(settings.get_bool(key_framerate_tk90x_60hz,false));
	mmu			= new MmuZxsp(this);
	keyboard	= new KeyboardZxsp(this,isa_KbdTk90x);
	//ay		=
	joystick	= new Tk90xJoy(this);
	//fdc		=
	//printer	=
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
