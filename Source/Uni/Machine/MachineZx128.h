#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxsp.h"
#include "ZxInfo/info.h"


class MachineZx128 : public MachineZxsp
{
protected:
	MachineZx128(IMachineController*, Model, isa_id id);

public:
	MachineZx128(IMachineController*, Model);
};
