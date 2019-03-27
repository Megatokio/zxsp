/*	Copyright  (c)	Günter Woigk 2006 - 2018
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

#include "KempstonMouse.h"
#include "Machine.h"
#include "MachineController.h"
#include "Qt/Screen/Screen.h"



/*	acc. to schematics on k1.spdns.de:

		$FBDF	%----.-011.--0-.----	x coord
		$FFDF	%----.-111.--0-.----	y coord
		$FADF	%----.--10.--0-.----	buttons: D0/D1 = right/left; 0 = pressed

	there are also other informations:

		middle button = D2 ((existed such a version?))
		address decoding A5 + full decoding of A8 to A11 for all: x, y, and buttons.
*/


//	WoS:
//	#define P_KMOUSE_BUTTONS		0xfadf  /* Port address */
//	#define B_KMOUSE_BUTTONS		0x0120  /* ---- ---0 --0- ---- */
//	#define P_KMOUSE_X				0xfbdf  /* Port address */
//	#define B_KMOUSE_X				0x0520  /* ---- -0-1 --0- ---- */
//	#define P_KMOUSE_Y				0xffdf  /* Port address */
//	#define B_KMOUSE_Y				0x0520  /* ---- -1-1 --0- ---- */
//
//	Horizontal position: IN 64479
//	Vertical postition:  IN 65503
//	Buttons: IN 64223 [255 = None], [254 = Left], [253 = Right], [252 = Both]



#define o_addr	NULL
#define	i_addr	"----.--1-.--0-.----"


KempstonMouse::KempstonMouse(Machine*m)
:
	Item(m,isa_KempstonMouse,isa_Mouse,external,o_addr,i_addr),
	scale(2),
	x(0),
	y(0)
{
	xlogIn("new KempstonMouse");
}


KempstonMouse::~KempstonMouse()
{
	xlogIn("~KempstonMouse");
	mouse.ungrab();
}


void KempstonMouse::powerOn( int32 cc )
{
	Item::powerOn(cc);

//	if(machine->controller->isFullScreen())		//	da müssen wir erst wieder die rechte Maustaste abfangen
//		mouse->grab(machine->controller);		//	außerdem bräuchten wir wohl einen Menüeintrag für "Grab Mouse"
}												//	wobei grabMouse(QWidget*) nur *über* dem Grabber-Widget funktioniert...


void KempstonMouse::input( Time, int32, uint16 address, uint8& byte, uint8& mask )
{
	switch( (address>>8)&7 )
	{
	case 6:	// read buttons:
	case 2:	// note: A10 not decoded for buttons
		byte &= getButtons();
		mask |= 3;				// 2-button version
		return;

	case 3:	// read x-axis:
		byte &= getXPos();
		mask  = 0xff;			// all bits driven
		return;

	case 7:	// read y-axis:
		byte &= getYPos();
		mask  = 0xff;			// all bits driven
		return;

	default:
		return;
	}
}

