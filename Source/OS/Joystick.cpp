// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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









