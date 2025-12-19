#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Grafpad.h"
#include "Inspector.h"

namespace zxsp
{

class GrafPadInsp : public Inspector
{
public:
	GrafPadInsp(QWidget*, MachineController*, volatile GrafPad*);
};

} // namespace zxsp
