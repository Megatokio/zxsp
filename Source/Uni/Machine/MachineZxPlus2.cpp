// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineZxPlus2.h"
#include "Ay/AySubclasses.h"
#include "Joy/SinclairJoy.h"
#include "Keyboard.h"
#include "MachineZx128.h"
#include "TapeRecorder.h"
#include "Ula/Mmu128k.h"
#include "Ula/Ula128k.h"


MachineZxPlus2::MachineZxPlus2(gui::MachineController* m, Model model) : MachineZx128(m, model, isa_MachineZxPlus2)
{
	assert(model == zxplus2 || model == zxplus2_span || model == zxplus2_frz);

	cpu		 = new Z80(this);	  // must be 1st item
	ula		 = new Ula128k(this); // should be 2nd item
	mmu		 = new Mmu128k(this);
	keyboard = new KeyboardZxPlus(this);
	ay		 = new AyForZx128(this);
	joystick = new ZxPlus2Joy(this);
	// fdc		=
	// printer	=
	taperecorder = new Plus2TapeRecorder(this);
}
