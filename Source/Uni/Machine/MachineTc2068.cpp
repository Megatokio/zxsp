/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

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










