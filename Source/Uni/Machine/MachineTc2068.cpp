// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineTc2068.h"
#include "ZxInfo.h"
#include "Z80/Z80.h"
#include "Ula/MmuTc2068.h"
#include "Ula/UlaTc2048.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"
#include "Joy/Tc2068Joy.h"


MachineTc2068::MachineTc2068(MachineController*m, Model model)
:
	MachineTc2048(m,model,isa_MachineTc2068)
{
	cpu			= new Z80(this);						// must be 1st item
	ula			= new UlaTc2048(this,isa_UlaTc2068);	// should be 2nd item
	mmu			= new MmuTc2068(this,isa_MmuTc2068);
	keyboard	= new KeyboardTimex(this);
	ay			= new AyForTc2068(this);
	joystick	= new Tc2068Joy(this);					// 2 x connected to AY chip
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}

void MachineTc2068::insertCartridge(cstr fpath)
{
	powerOff();
		NV(MmuTc2068Ptr(mmu))->insertCartridge(fpath);
	powerOn();
}










