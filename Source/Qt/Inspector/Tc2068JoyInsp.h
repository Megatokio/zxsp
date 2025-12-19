// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy/Tc2068Joy.h"
#include "JoyInsp.h"

namespace zxsp
{

class Tc2068JoyInsp : public JoyInsp
{
	volatile Tc2068Joy* const tc2068joy;

public:
	Tc2068JoyInsp(QWidget*, MachineController*, volatile Tc2068Joy*, cstr img_path);

protected:
	cstr lineedit_text(uint port, uint8 state) override;
};

} // namespace zxsp
