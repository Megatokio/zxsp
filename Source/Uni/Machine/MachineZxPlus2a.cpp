/*	Copyright  (c)	Günter Woigk 1995 - 2018
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

#include "MachineZxPlus2a.h"
#include "Ula/UlaPlus3.h"
#include "Ula/MmuPlus3.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"


MachineZxPlus2a::MachineZxPlus2a(MachineController* m, Model model, isa_id id)
:	MachineZx128(m,model,id) {}

MachineZxPlus2a::MachineZxPlus2a(MachineController* m, Model model)
:
	MachineZx128(m,model,isa_MachineZxPlus2a)
{
	assert(model==zxplus2a || model==zxplus2a_span);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaPlus3(this);	// should be 2nd item
	mmu			= new MmuPlus3(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	joystick	= new ZxPlus2AJoy(this);
	//fdc		=
	printer		= new PrinterPlus3(this);
	taperecorder = new Plus2aTapeRecorder(this);
}


