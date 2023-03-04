// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "KempstonMouse.h"
#include "Machine.h"
#include "MachineController.h"


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


#define o_addr nullptr
#define i_addr "----.--1-.--0-.----"


KempstonMouse::KempstonMouse(Machine* m) :
	Item(m, isa_KempstonMouse, isa_Mouse, external, o_addr, i_addr),
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


void KempstonMouse::powerOn(int32 cc)
{
	Item::powerOn(cc);

	//	if(machine->controller->isFullScreen())		//	da müssen wir erst wieder die rechte Maustaste abfangen
	//		mouse->grab(machine->controller);		//	außerdem bräuchten wir wohl einen Menüeintrag für "Grab Mouse"
} //	wobei grabMouse(QWidget*) nur *über* dem Grabber-Widget funktioniert...


void KempstonMouse::input(Time, int32, uint16 address, uint8& byte, uint8& mask)
{
	switch ((address >> 8) & 7)
	{
	case 6: // read buttons:
	case 2: // note: A10 not decoded for buttons
		byte &= getButtons();
		mask |= 3; // 2-button version
		return;

	case 3: // read x-axis:
		byte &= getXPos();
		mask = 0xff; // all bits driven
		return;

	case 7: // read y-axis:
		byte &= getYPos();
		mask = 0xff; // all bits driven
		return;

	default: return;
	}
}

void KempstonMouse::setScale(int n)
{
	x	  = x / scale * n;
	y	  = y / scale * n;
	scale = n;
}

uint8 KempstonMouse::getXPos()
{
	if (machine == front_machine)
	{
		int dx = mouse.dx;
		mouse.dx -= dx;
		x += dx;
	}
	return uint8(x / scale);
}

uint8 KempstonMouse::getYPos()
{
	if (machine == front_machine)
	{
		int dy = mouse.dy;
		mouse.dy -= dy;
		y += dy;
	}
	return uint8(y / scale);
}

uint8 KempstonMouse::getButtons() const
{
	return mouse.isGrabbed() && machine == front_machine ?
			   0xff - (mouse.getLeftButton() * 2) - mouse.getRightButton() : // 2-button version
			   0xff;
}
