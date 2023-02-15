// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTs1000.h"
#include "Keyboard.h"
#include "MachineZx81.h"
#include "TapeRecorder.h"


MachineTs1000::MachineTs1000(MachineController* m) : MachineZx81(m, isa_MachineTs1000, ts1000)
{
	cpu		 = new Z80(this);	  // must be 1st item
	ula		 = new UlaZx81(this); // should be 2nd item
	mmu		 = new MmuZx81(this);
	keyboard = new KeyboardZx81(this, isa_KbdTs1000);
	// ay		=
	// joystick	=
	// fdc		=
	// printer	=
	taperecorder = new TS2020(this);
}
