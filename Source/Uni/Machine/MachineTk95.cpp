/*	Copyright  (c)	Günter Woigk 2012 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/


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
