#pragma once
// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/InvesJoy.h"
#include "JoyInsp.h"

namespace zxsp
{

class InvesJoyInsp : public JoyInsp
{
public:
	InvesJoyInsp(QWidget*, MachineController*, volatile InvesJoy*);
};

} // namespace zxsp
