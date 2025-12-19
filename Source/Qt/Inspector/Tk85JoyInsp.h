// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy/Tk85Joy.h"
#include "JoyInsp.h"

namespace zxsp
{

class Tk85JoyInsp : public JoyInsp
{
public:
	Tk85JoyInsp(QWidget*, MachineController*, volatile Tk85Joy*);
};

} // namespace zxsp
