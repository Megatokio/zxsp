// Copyright (c) 2019 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Dsp.h"
#include "kio/kio.h"

namespace Dsp
{
void startCoreAudio(bool input_enabled)
{
	debugstr("startCoreAudio\n");
	TODO();
}
void stopCoreAudio() { debugstr("stopCoreAudio\n"); }
} // namespace Dsp


#include "Mouse.h"
Mouse mouse;
Mouse::Mouse() : dx(0), dy(0) { debugstr("Mouse\n"); }
Mouse::~Mouse() { debugstr("~Mouse"); }
void Mouse::grab(QWidget*) { debugstr("Mouse::grab\n"); }
void Mouse::ungrab() { debugstr("Mouase::ungrab\n"); }


#include "Joystick.h"
void findUsbJoysticks() { debugstr("findUsbJoysticks"); }
