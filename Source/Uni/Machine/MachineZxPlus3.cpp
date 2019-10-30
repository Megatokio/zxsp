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

#include "MachineZxPlus3.h"
#include "Ula/UlaPlus3.h"
#include "Ula/MmuPlus3.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"


MachineZxPlus3::MachineZxPlus3(MachineController*m, Model model)
:
	MachineZxPlus2a(m,model,isa_MachineZxPlus3)
{
	assert(model==zxplus3 || model==zxplus3_span);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaPlus3(this);	// should be 2nd item
	mmu			= new MmuPlus3(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	joystick	= new ZxPlus3Joy(this);
	fdc			= new FdcPlus3(this);
	printer		= new PrinterPlus3(this);
	taperecorder = new Walkman(this);
}

void MachineZxPlus3::insertDisk( cstr fpath, char side )
{
	bool f = suspend();
		fdc->getDrive(0)->insertDisk(fpath, (side|0x20) == 'b');
	if(f) resume();
}


