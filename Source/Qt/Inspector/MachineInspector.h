// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"

namespace zxsp
{
class MachineInspector : public Inspector
{
public:
	MachineInspector(QWidget*, MachineController*, volatile Machine*);
};
} // namespace zxsp
