#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/Tc2048Joy.h"
#include "JoyInsp.h"

namespace gui
{

class Tc2048JoyInsp : public JoyInsp
{
public:
	Tc2048JoyInsp(QWidget*, MachineController*, volatile Tc2048Joy*);
};

} // namespace gui
