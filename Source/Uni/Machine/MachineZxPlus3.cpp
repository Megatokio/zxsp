// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxPlus3.h"
#include "Ay/AySubclasses.h"
#include "Fdc/FdcPlus3.h"
#include "Keyboard.h"
#include "Printer/PrinterPlus3.h"
#include "TapeRecorder.h"
#include "Ula/MmuPlus3.h"
#include "Ula/UlaPlus3.h"

MachineZxPlus3::MachineZxPlus3(IMachineController* m, Model model) : MachineZxPlus2a(m, model, isa_MachineZxPlus3)
{
	assert(model == zxplus3 || model == zxplus3_span);

	addItem(new Z80(this));		 // must be 1st item
	addItem(new UlaPlus3(this)); // should be 2nd item
	addItem(new MmuPlus3(this));
	addItem(new KeyboardZxPlus(this));
	addItem(new AyForZx128(this));
	addItem(new ZxPlus3Joy(this));
	addItem(new FdcPlus3(this));
	addItem(new PrinterPlus3(this));
	addItem(new Walkman(this));
}

void MachineZxPlus3::insertDisk(cstr fpath, char side)
{
	bool f = suspend();
	fdc->getDrive(0)->insertDisk(fpath, (side | 0x20) == 'b');
	if (f) resume();
}
