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


#include "MachineInves.h"
#include "MachineZxsp.h"
#include "Ula/UlaInves.h"
#include "Ula/MmuInves.h"
#include "Joy/InvesJoy.h"
#include "Keyboard.h"
#include "TapeRecorder.h"


MachineInves::MachineInves(MachineController*m)
:
	MachineZxsp(m,inves,isa_MachineInves)
{
	cpu			= new Z80(this);		// must be 1st item
	ula			= new UlaInves(this);	// should be 2nd item
	mmu			= new MmuInves(this);
	keyboard	= new KeyboardZxPlus(this);
	//ay		=
	joystick	= new InvesJoy(this);
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}
