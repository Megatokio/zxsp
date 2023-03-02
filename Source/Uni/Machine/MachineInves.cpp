// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineInves.h"
#include "Joy/InvesJoy.h"
#include "Keyboard.h"
#include "MachineZxsp.h"
#include "TapeRecorder.h"
#include "Ula/MmuInves.h"
#include "Ula/UlaInves.h"


MachineInves::MachineInves(gui::MachineController* m) : MachineZxsp(m, inves, isa_MachineInves)
{
	addItem(new Z80(this));		 // must be 1st item
	addItem(new UlaInves(this)); // should be 2nd item
	addItem(new MmuInves(this));
	addItem(new KeyboardZxPlus(this));
	addItem(new InvesJoy(this));
	addItem(new Walkman(this));
}
