/*	Copyright  (c)	Günter Woigk 2016 - 2019
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
