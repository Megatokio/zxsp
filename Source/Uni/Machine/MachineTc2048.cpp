/*	Copyright  (c)	Günter Woigk 2009 - 2018
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

#include "MachineTc2048.h"
#include "TapeRecorder.h"
#include "Keyboard.h"
#include "Ula/UlaTc2048.h"
#include "Ula/MmuTc2048.h"
#include "Joy/Tc2048Joy.h"


/*
the main difference is that instead of ULA there is another chip, SCLD.
Besides all the ULA functions SCLD is able to work in some other video
modes. Unlike Spectrum in TIMEX port #FE is fully decoded, what in rare
cases is causing incompatibility. The other hardware difference is joy-
stick port (Kempston) built in TIMEX. ROM code is the same as in Spectrum
except one OUT instruction setting proper video mode after reset.
*/


MachineTc2048::MachineTc2048(MachineController*m, Model model, isa_id id)
:	MachineZxsp(m,model,id)
{}

MachineTc2048::MachineTc2048(MachineController*m)
:
	MachineZxsp(m,tc2048,isa_MachineTc2048)
{
	cpu			= new Z80(this);						// must be 1st item
	ula			= new UlaTc2048(this,isa_UlaTc2048);	// should be 2nd item
	mmu			= new MmuTc2048(this);
	keyboard	= new KeyboardTimex(this);
	//ay		=
	joystick	= new Tc2048Joy(this);
	//fdc		=
	//printer	=
	taperecorder = new Walkman(this);
}

void MachineTc2048::loadScr(FD& fd) throws
{
	ula->setPortFF(ula->getPortFF() & 0x3F);	// reset video-related bits
	MachineZxsp::loadScr(fd);
}




