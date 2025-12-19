// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"
#include "ZxIf1.h"

namespace zxsp
{

class ZxIf1Insp : public Inspector
{
public:
	ZxIf1Insp(QWidget*, MachineController*, volatile ZxIf1*);
};

} // namespace zxsp
