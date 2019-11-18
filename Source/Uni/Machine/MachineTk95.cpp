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


#include "MachineTk95.h"
#include "MachineZxsp.h"
#include "Settings.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Joy/SinclairJoy.h"


/* A later model, the TK-95, which boasted an improved keyboard
   (similar to the Commodore 64's) and a more compatible ROM,
   was little more than a Timex TC2048 (another Spectrum
   clone) in disguise.

wikipedia:
	The TK90X was replaced by the TK95, which had a different keyboard (professional)
	and case (identical to Commodore Plus4) and exactly the same circuit board and schematics.
	The motherboard was marked as TK90X.

kio: so i believe this TK90X info also applies:
	the TV system was hardware selectable to PAL-M (60 Hz) as used in Brazil,
	PAL-N (50 Hz) as used in Uruguay, Argentina and Paraguay
	and NTSC (60 Hz) as used in USA and many other countries.
*/

MachineTk95::MachineTk95(MachineController*m)
:
	MachineZxsp(m,tk95,isa_MachineTk95)
{
	cpu			= new Z80(this);
	ula			= new UlaTk90x(this); ula->set60Hz(settings.get_bool(key_framerate_tk95_60hz,false));
	mmu			= new MmuZxsp(this);
	keyboard	= new KeyboardZxsp(this,isa_KbdTk95);
	//ay		=
	joystick	= new Tk95Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);
}
