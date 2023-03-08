// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"
#include "Machine/Machine.h"

Joy::Joy(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr, cstr idf1, cstr idf2, cstr idf3) :
	Item(m, id, isa_Joy, internal, o_addr, i_addr),
	joystick_idf {idf1, idf2, idf3},
	num_ports(
		idf3 ? 3 :
		idf2 ? 2 :
			   1)
{
	xlogIn("new Joy");

	// attach kbd_joystick only on explicit request else "some keys do not react" disaster!

	insertJoystick(0, usb_joystick0);
	if (idf2) insertJoystick(1, usb_joystick1);
	if (idf3) insertJoystick(2, usb_yoystick2);
}

uint8 Joy::getButtonsFUDLR(uint i) const { return machine->getJoystickButtons(joystick_id[i]); }
