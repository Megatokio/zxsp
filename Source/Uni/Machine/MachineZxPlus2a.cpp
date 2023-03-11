// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxPlus2a.h"
#include "Ay/AySubclasses.h"
#include "Keyboard.h"
#include "Printer/PrinterPlus3.h"
#include "TapeRecorder.h"
#include "Ula/MmuPlus3.h"
#include "Ula/UlaPlus3.h"


MachineZxPlus2a::MachineZxPlus2a(IMachineController* m, Model model, isa_id id) : MachineZx128(m, model, id) {}

MachineZxPlus2a::MachineZxPlus2a(IMachineController* m, Model model) : MachineZx128(m, model, isa_MachineZxPlus2a)
{
	assert(model == zxplus2a || model == zxplus2a_span);

	addItem(new Z80(this));		 // must be 1st item
	addItem(new UlaPlus3(this)); // should be 2nd item
	addItem(new MmuPlus3(this));
	addItem(new KeyboardZxPlus(this));
	addItem(new AyForZx128(this));
	addItem(new ZxPlus2AJoy(this));
	addItem(new PrinterPlus3(this));
	addItem(new Plus2aTapeRecorder(this));
}
