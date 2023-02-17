// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTk85.h"
#include "Joy/Tk85Joy.h"
#include "Keyboard.h"
#include "Settings.h"
#include "TapeRecorder.h"
#include "Ula/MmuTk85.h"


MachineTk85::MachineTk85(gui::MachineController* m) : MachineZx81(m, isa_MachineTk85, tk85)
{
	cpu = new Z80(this);
	ula = new UlaZx81(this);
	ula->set60Hz(gui::settings.get_bool(key_framerate_tk85_60hz, false));
	mmu		 = new MmuTk85(this);
	keyboard = new KeyboardZx81(this, isa_KbdTk85);
	// ay		=
	joystick = new Tk85Joy(this);
	// fdc		=
	// printer	=
	taperecorder = new TS2020(this);
}
