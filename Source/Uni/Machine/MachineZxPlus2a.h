#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/SinclairJoy.h"
#include "MachineZx128.h"
#include "Printer/PrinterPlus3.h"
#include "ZxInfo/info.h"


class MachineZxPlus2a : public MachineZx128
{
protected:
	MachineZxPlus2a(MachineController*, Model, isa_id);

public:
	MachineZxPlus2a(MachineController*, Model);
};
