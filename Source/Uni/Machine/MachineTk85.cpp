// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTk85.h"
#include "Joy/Tk85Joy.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/MmuTk85.h"


MachineTk85::MachineTk85(IMachineController* m, bool is60hz) : MachineZx81(m, isa_MachineTk85, tk85)
{
	addItem(new Z80(this));
	addItem(new UlaZx81(this));
	ula->set60Hz(is60hz);
	addItem(new MmuTk85(this));
	addItem(new KeyboardZx81(this, isa_KbdTk85));
	addItem(new Tk85Joy(this));
	addItem(new TS2020(this));
}
