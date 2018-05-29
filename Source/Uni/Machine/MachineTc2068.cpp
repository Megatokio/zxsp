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










