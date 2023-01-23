// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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


