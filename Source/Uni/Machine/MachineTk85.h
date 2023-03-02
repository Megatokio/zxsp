#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZx81.h"


class MachineTk85 : public MachineZx81
{
public:
	explicit MachineTk85(gui::MachineController*, bool is60hz = false);
};
