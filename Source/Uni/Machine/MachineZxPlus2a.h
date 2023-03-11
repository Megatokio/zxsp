#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZx128.h"


class MachineZxPlus2a : public MachineZx128
{
protected:
	MachineZxPlus2a(IMachineController*, Model, isa_id);

public:
	MachineZxPlus2a(IMachineController*, Model);
};
