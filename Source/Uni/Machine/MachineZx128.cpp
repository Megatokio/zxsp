// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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




