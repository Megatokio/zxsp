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


#include "MachineZxPlus2.h"
#include "MachineZx128.h"
#include "Ula/Ula128k.h"
#include "Ula/Mmu128k.h"
#include "Keyboard.h"
#include "TapeRecorder.h"
#include "Ay/AySubclasses.h"
#include "Joy/SinclairJoy.h"


MachineZxPlus2::MachineZxPlus2(MachineController*m, Model model)
:
	MachineZx128(m,model,isa_MachineZxPlus2)
{
	assert(model==zxplus2 || model==zxplus2_span || model==zxplus2_frz);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new Ula128k(this);	// should be 2nd item
	mmu			= new Mmu128k(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	joystick	= new ZxPlus2Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new Plus2TapeRecorder(this);
}


