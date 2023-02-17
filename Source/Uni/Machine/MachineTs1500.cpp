// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTs1500.h"
#include "Keyboard.h"
#include "MachineZx81.h"
#include "TapeRecorder.h"
#include "Ula/MmuTs1500.h"


MachineTs1500::MachineTs1500(gui::MachineController* m) : MachineZx81(m, isa_MachineTs1500, ts1500)
{
	cpu		 = new Z80(this);	  // must be 1st item
	ula		 = new UlaZx81(this); // should be 2nd item
	mmu		 = new MmuTs1500(this);
	keyboard = new KeyboardZx81(this, isa_KbdTs1500);
	// ay		=
	// joystick	=
	// fdc		=
	// printer	=
	taperecorder = new TS2020(this);
}
