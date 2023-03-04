// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTs1000.h"
#include "Keyboard.h"
#include "MachineZx81.h"
#include "TapeRecorder.h"


MachineTs1000::MachineTs1000(IMachineController* m) : MachineZx81(m, isa_MachineTs1000, ts1000)
{
	addItem(new Z80(this));		// must be 1st item
	addItem(new UlaZx81(this)); // should be 2nd item
	addItem(new MmuZx81(this));
	addItem(new KeyboardZx81(this, isa_KbdTs1000));
	addItem(new TS2020(this));
}
