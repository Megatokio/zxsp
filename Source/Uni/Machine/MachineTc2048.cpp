/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include "MachineTc2048.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ula/UlaTc2048.h"
#include "Ula/MmuTc2048.h"
#include "Joy/Tc2048Joy.h"


/*
the main difference is that instead of ULA there is another chip, SCLD.
Besides all the ULA functions SCLD is able to work in some other video
modes. Unlike Spectrum in TIMEX port #FE is fully decoded, what in rare
cases is causing incompatibility. The other hardware difference is joy-
stick port (Kempston) built in TIMEX. ROM code is the same as in Spectrum
except one OUT instruction setting proper video mode after reset.
*/


MachineTc2048::MachineTc2048(MachineController*m, Model model, isa_id id)
:	MachineZxsp(m,model,id)
{}

MachineTc2048::MachineTc2048(MachineController*m)
:
	MachineZxsp(m,tc2048,isa_MachineTc2048)
{
	cpu			= new Z80(this);						// must be 1st item
	ula			= new UlaTc2048(this,isa_UlaTc2048);	// should be 2nd item
	mmu			= new MmuTc2048(this);
	keyboard	= new KeyboardTimex(this);
	//ay		=
	joystick	= new Tc2048Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}

void MachineTc2048::loadScr(FD& fd) throws
{
	ula->setPortFF(ula->getPortFF() & 0x3F);	// reset video-related bits
	MachineZxsp::loadScr(fd);
}




