/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "MachineZxPlus2a.h"
#include "Ula/UlaPlus3.h"
#include "Ula/MmuPlus3.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"


MachineZxPlus2a::MachineZxPlus2a(MachineController* m, Model model, isa_id id)
:	MachineZx128(m,model,id) {}

MachineZxPlus2a::MachineZxPlus2a(MachineController* m, Model model)
:
	MachineZx128(m,model,isa_MachineZxPlus2a)
{
	assert(model==zxplus2a || model==zxplus2a_span);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaPlus3(this);	// should be 2nd item
	mmu			= new MmuPlus3(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	joystick	= new ZxPlus2AJoy(this);
	//fdc		=
	printer		= new PrinterPlus3(this);
	taperecorder = new Plus2aTapeRecorder(this);
}


