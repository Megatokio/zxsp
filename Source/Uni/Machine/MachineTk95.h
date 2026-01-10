// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineZxsp.h"


namespace zxsp
{

class MachineTk95 : public MachineZxsp
{
public:
	explicit MachineTk95(IMachineController*, IScreen*, bool is60hz = false);
};

} // namespace zxsp
