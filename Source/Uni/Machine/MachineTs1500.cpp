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


#include "MachineTs1500.h"
#include "MachineZx81.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/MmuTs1500.h"


MachineTs1500::MachineTs1500(MachineController*m)
:
	MachineZx81(m,isa_MachineTs1500,ts1500)
{
	cpu			= new Z80(this);			// must be 1st item
	ula			= new UlaZx81(this);		// should be 2nd item
	mmu			= new MmuTs1500(this);
	keyboard	= new KeyboardZx81(this,isa_KbdTs1500);
	//ay		=
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);
}


