// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UsbJoystick.h"

UsbJoystick::UsbJoystick() : Joystick(isa_UsbJoystick)
{
	xlogIn("new UsbJoystick"); //
}
