// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTs1500.h"
#include "Keyboard.h"
#include "MachineZx81.h"
#include "TapeRecorder.h"
#include "Ula/MmuTs1500.h"


namespace zxsp
{

MachineTs1500::MachineTs1500(IMachineController* m, IScreen* screen) : //
	MachineZx81(m, screen, isa_MachineTs1500, ts1500)
{
	addItem(new Z80(this));		// must be 1st item
	addItem(new UlaZx81(this)); // should be 2nd item
	addItem(new MmuTs1500(this));
	addItem(new KeyboardZx81(this, isa_KbdTs1500));
	addItem(new TS2020(this));
}

} // namespace zxsp
