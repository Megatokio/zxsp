#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxsp.h"


class MachineTk95 : public MachineZxsp
{
public:
	explicit MachineTk95(IMachineController*, bool is60hz = false);
};
