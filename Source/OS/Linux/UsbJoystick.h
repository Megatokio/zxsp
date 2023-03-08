// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once

#include "Joystick.h"

class UsbJoystick final : public Joystick
{
public:
	UsbJoystick();
	uint8 getState(bool) const volatile override { return 0; }
	bool  isConnected() const volatile override { return yes; }
};
