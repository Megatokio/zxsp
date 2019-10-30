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

#include "MachineZx128.h"
#include "ZxInfo.h"
#include "Z80/Z80.h"
#include "Item.h"
#include "Ula/Ula128k.h"
#include "Ula/Mmu128k.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ay/AySubclasses.h"


MachineZx128::MachineZx128(MachineController* m, Model model, isa_id id)
:MachineZxsp(m,model,id){}

MachineZx128::MachineZx128(MachineController* m, Model model)
:
	MachineZxsp(m,model,zx_info[model].id)
{
	assert(model==zx128 || model==zx128_span);
	assert(model_info->id==isa_MachineZx128);

	cpu			= new Z80(this);		// must be 1st item
	ula			= new Ula128k(this);	// should be 2nd item
	mmu			= new Mmu128k(this);
	keyboard	= new KeyboardZxPlus(this);
	ay			= new AyForZx128(this);
	//joystick	=
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}




