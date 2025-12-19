// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineZx128.h"


namespace zxsp
{

class MachineZxPlus2a : public MachineZx128
{
protected:
	MachineZxPlus2a(IMachineController*, Model, isa_id);

public:
	MachineZxPlus2a(IMachineController*, Model);
};

} // namespace zxsp
