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


MachineZxPlus2::MachineZxPlus2(IMachineController* m, Model model) : MachineZx128(m, model, isa_MachineZxPlus2)
{
	assert(model == zxplus2 || model == zxplus2_span || model == zxplus2_frz);

	addItem(new Z80(this));		// must be 1st item
	addItem(new Ula128k(this)); // should be 2nd item
	addItem(new Mmu128k(this));
	addItem(new KeyboardZxPlus(this));
	addItem(new AyForZx128(this));
	addItem(new ZxPlus2Joy(this));
	addItem(new Plus2TapeRecorder(this));
}
