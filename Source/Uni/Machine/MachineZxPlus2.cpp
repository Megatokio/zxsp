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


#include "MachineZxPlus2.h"
#include "MachineZx128.h"
#include "Ula/Ula128k.h"
#include "Ula/Mmu128k.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ay/AySubclasses.h"
#include "Joy/SinclairJoy.h"


MachineZxPlus2::MachineZxPlus2(MachineController*m, Model model)
:
	MachineZx128(m,model,isa_MachineZxPlus2)
{
	assert(model==zxplus2 || model==zxplus2_span || model==zxplus2_frz);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new Ula128k(this);	// should be 2nd item
	mmu			= new Mmu128k(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	joystick	= new ZxPlus2Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new Plus2TapeRecorder(this);
}



