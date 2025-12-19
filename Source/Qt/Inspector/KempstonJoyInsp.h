#pragma once
// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/KempstonJoy.h"
#include "JoyInsp.h"

namespace zxsp
{

class KempstonJoyInsp : public JoyInsp
{
public:
	KempstonJoyInsp(QWidget*, MachineController*, volatile KempstonJoy*);
};

} // namespace zxsp
