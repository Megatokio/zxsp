// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineTc2048.h"
#include "Joy/Tc2048Joy.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ula/MmuTc2048.h"
#include "Ula/UlaTc2048.h"


/*
the main difference is that instead of ULA there is another chip, SCLD.
Besides all the ULA functions SCLD is able to work in some other video
modes. Unlike Spectrum in TIMEX port #FE is fully decoded, what in rare
cases is causing incompatibility. The other hardware difference is joy-
stick port (Kempston) built in TIMEX. ROM code is the same as in Spectrum
except one OUT instruction setting proper video mode after reset.
*/


MachineTc2048::MachineTc2048(IMachineController* m, Model model, isa_id id) : MachineZxsp(m, model, id) {}

MachineTc2048::MachineTc2048(IMachineController* m) : MachineZxsp(m, tc2048, isa_MachineTc2048)
{
	addItem(new Z80(this));						 // must be 1st item
	addItem(new UlaTc2048(this, isa_UlaTc2048)); // should be 2nd item
	addItem(new MmuTc2048(this));
	addItem(new KeyboardTimex(this, isa_KbdTimex));
	addItem(new Tc2048Joy(this));
	addItem(new Walkman(this));
}

void MachineTc2048::loadScr(FD& fd)
{
	ula->setPortFF(ula->getPortFF() & 0x3F); // reset video-related bits
	MachineZxsp::loadScr(fd);
}
