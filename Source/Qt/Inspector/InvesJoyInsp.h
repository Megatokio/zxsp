#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/InvesJoy.h"
#include "JoyInsp.h"

namespace gui
{

class InvesJoyInsp : public JoyInsp
{
public:
	InvesJoyInsp(QWidget*, MachineController*, volatile InvesJoy*);
};

} // namespace gui
