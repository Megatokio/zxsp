// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MachineTk95.h"
#include "Joy/SinclairJoy.h"
#include "Keyboard.h"
#include "MachineZxsp.h"
#include "TapeRecorder.h"
#include "Ula/MmuZxsp.h"
#include "Ula/UlaZxsp.h"


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

MachineTk95::MachineTk95(IMachineController* m, bool is60hz) : MachineZxsp(m, tk95, isa_MachineTk95)
{
	addItem(new Z80(this));
	addItem(new UlaTk90x(this, is60hz));
	addItem(new MmuZxsp(this));
	addItem(new KeyboardZxsp(this, isa_KbdTk95));
	addItem(new Tk95Joy(this));
	addItem(new TS2020(this));
}
