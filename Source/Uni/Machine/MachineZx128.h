// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineZxsp.h"


namespace zxsp
{

class MachineZx128 : public MachineZxsp
{
protected:
	MachineZx128(IMachineController*, Model, isa_id id);

public:
	MachineZx128(IMachineController*, Model);
};

} // namespace zxsp
