#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/Tc2068Joy.h"
#include "JoyInsp.h"

namespace gui
{

class Tc2068JoyInsp : public JoyInsp
{
	volatile Tc2068Joy* const tc2068joy;

public:
	Tc2068JoyInsp(QWidget*, MachineController*, volatile Tc2068Joy*, cstr img_path);

protected:
	void updateWidgets() override;
};

} // namespace gui
