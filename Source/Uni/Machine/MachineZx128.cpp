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

#include "MachineZx128.h"
#include "ZxInfo.h"
#include "Z80/Z80.h"
#include "Item.h"
#include "Ula/Ula128k.h"
#include "Ula/Mmu128k.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"


MachineZx128::MachineZx128(MachineController* m, Model model, isa_id id)
:MachineZxsp(m,model,id){}

MachineZx128::MachineZx128(MachineController* m, Model model)
:
	MachineZxsp(m,model,zx_info[model].id)
{
	assert(model==zx128 || model==zx128_span);
	assert(model_info->id==isa_MachineZx128);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new Ula128k(this);	// should be 2nd item
	mmu			= new Mmu128k(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}




