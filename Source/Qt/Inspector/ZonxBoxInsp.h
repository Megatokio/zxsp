// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"

namespace zxsp
{

class ZonxBoxInsp : public Inspector
{
public:
	ZonxBoxInsp(QWidget*, MachineController* mc, volatile Ay*);
};

} // namespace zxsp
