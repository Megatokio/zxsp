/*	Copyright  (c)	Günter Woigk 2006 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

//#include "UsbJoystick.h"
#include "Joystick.h"



static_assert(max_joy == 5, "max_joy != 5");
static_assert(num_usb == 3, "num_usb != 3");

Joystick* joysticks[max_joy]; // = {0,0,0,0,0};

ON_INIT([]
{
	joysticks[kbd_joystick] = new KbdJoystick();
	joysticks[no_joystick]  = new NoJoystick();
	findUsbJoysticks();
});









