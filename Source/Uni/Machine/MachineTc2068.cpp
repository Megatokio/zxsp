// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineTc2068.h"
#include "Ay/AySubclasses.h"
#include "Joy/Tc2068Joy.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/MmuTc2068.h"
#include "Ula/UlaTc2048.h"
#include "Z80/Z80.h"
#include "ZxInfo.h"


MachineTc2068::MachineTc2068(IMachineController* m, Model model) : MachineTc2048(m, model, isa_MachineTc2068)
{
	addItem(new Z80(this));							// must be 1st item
	addItem(new UlaTc2048(this, isa_UlaTc2068));	// should be 2nd item
	addItem(new MmuTc2068(this, isa_MmuTc2068));	//
	addItem(new KeyboardTimex(this, isa_KbdTimex)); //
	Tc2068Joy* joy = new Tc2068Joy(this);			// 2 x connected to AY chip
	addItem(joy);
	addItem(new AyForTc2068(this, joy));
	addItem(new Walkman(this));
}

void MachineTc2068::insertCartridge(cstr fpath)
{
	// called from MachineController.loadSnapshot()

	auto* dock = dynamic_cast<MmuTc2068*>(mmu);
	assert(dock);

	bool f = powerOff();
	dock->insertCartridge(fpath);
	if (f) powerOn();
}
