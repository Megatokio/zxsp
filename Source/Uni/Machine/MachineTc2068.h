#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineTc2048.h"


class MachineTc2068 : public MachineTc2048
{
public:
	MachineTc2068(gui::MachineController*, Model);

	void insertCartridge(cstr fpath);
};
