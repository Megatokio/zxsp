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


#include "MachineTk85.h"
#include "Settings.h"
#include "Ula/MmuTk85.h"
#include "Joy/Tk85Joy.h"
#include "Keyboard.h"
#include "TapeRecorder.h"


MachineTk85::MachineTk85(MachineController *m)
:
	MachineZx81(m,isa_MachineTk85,tk85)
{
	cpu			= new Z80(this);
	ula			= new UlaZx81(this); ula->set60Hz(settings.get_bool(key_framerate_tk85_60hz,false));
	mmu			= new MmuTk85(this);
	keyboard	= new KeyboardZx81(this,isa_KbdTk85);
	//ay		=
	joystick	= new Tk85Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);
}

