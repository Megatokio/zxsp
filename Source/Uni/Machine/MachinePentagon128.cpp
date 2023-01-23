// Copyright (c) 2016 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "MachinePentagon128.h"
#include "MachineZx128.h"
#include "Ula/Ula128k.h"
#include "Ula/Mmu128k.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ay/AySubclasses.h"
#include "Joy/Tc2048Joy.h"


MachinePentagon128::MachinePentagon128(MachineController* mc)
:
	MachineZx128(mc,pentagon128,isa_MachinePentagon128)
{
	cpu			= new Z80(this);			// must be 1st item
	ula			= new Ula128k(this);		// should be 2nd item
	mmu			= new Mmu128k(this);		// TODO: verify!
	keyboard	= new KeyboardZxPlus(this);	// TODO: use own image!
	ay			= new AyForZx128(this);
	joystick	= new Tc2048Joy(this);		// TODO: use own image!
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}
