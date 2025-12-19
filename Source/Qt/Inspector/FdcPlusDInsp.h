#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcPlusD.h"
#include "JoyInsp.h"

namespace zxsp
{

class FdcPlusDInsp : public Inspector
{
public:
	FdcPlusDInsp(QWidget*, MachineController*, volatile FdcPlusD*);
};

} // namespace zxsp
