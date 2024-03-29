#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/Tk85Joy.h"
#include "JoyInsp.h"

namespace gui
{

class Tk85JoyInsp : public JoyInsp
{
public:
	Tk85JoyInsp(QWidget*, MachineController*, volatile Tk85Joy*);
};

} // namespace gui
