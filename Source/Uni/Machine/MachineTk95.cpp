// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTk95.h"
#include "MachineZxsp.h"
#include "Settings.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Joy/SinclairJoy.h"


/* A later model, the TK-95, which boasted an improved keyboard
   (similar to the Commodore 64's) and a more compatible ROM,
   was little more than a Timex TC2048 (another Spectrum
   clone) in disguise.

wikipedia:
	The TK90X was replaced by the TK95, which had a different keyboard (professional)
	and case (identical to Commodore Plus4) and exactly the same circuit board and schematics.
	The motherboard was marked as TK90X.

kio: so i believe this TK90X info also applies:
	the TV system was hardware selectable to PAL-M (60 Hz) as used in Brazil,
	PAL-N (50 Hz) as used in Uruguay, Argentina and Paraguay
	and NTSC (60 Hz) as used in USA and many other countries.
*/

MachineTk95::MachineTk95(MachineController*m)
:
	MachineZxsp(m,tk95,isa_MachineTk95)
{
	cpu			= new Z80(this);
	ula			= new UlaTk90x(this); ula->set60Hz(settings.get_bool(key_framerate_tk95_60hz,false));
	mmu			= new MmuZxsp(this);
	keyboard	= new KeyboardZxsp(this,isa_KbdTk95);
	//ay		=
	joystick	= new Tk95Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new TS2020(this);
}
