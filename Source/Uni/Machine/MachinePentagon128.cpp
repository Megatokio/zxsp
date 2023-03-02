// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachinePentagon128.h"
#include "Ay/AySubclasses.h"
#include "Joy/Tc2048Joy.h"
#include "Keyboard.h"
#include "MachineZx128.h"
#include "TapeRecorder.h"
#include "Ula/Mmu128k.h"
#include "Ula/Ula128k.h"
#include "kio/kio.h"


MachinePentagon128::MachinePentagon128(gui::MachineController* mc) :
	MachineZx128(mc, pentagon128, isa_MachinePentagon128)
{
	addItem(new Z80(this));			   // must be 1st item
	addItem(new Ula128k(this));		   // should be 2nd item
	addItem(new Mmu128k(this));		   // TODO: verify!
	addItem(new KeyboardZxPlus(this)); // TODO: use own image!
	addItem(new AyForZx128(this));
	addItem(new Tc2048Joy(this)); // TODO: use own image!
	addItem(new Walkman(this));
}
