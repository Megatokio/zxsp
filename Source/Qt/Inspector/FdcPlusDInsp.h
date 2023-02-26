#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcPlusD.h"
#include "JoyInsp.h"

namespace gui
{

class FdcPlusDInsp : public Inspector
{
public:
	FdcPlusDInsp(QWidget*, MachineController*, volatile FdcPlusD*);
};

} // namespace gui
