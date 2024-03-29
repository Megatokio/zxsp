#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxPlus2a.h"


class MachineZxPlus3 : public MachineZxPlus2a
{
public:
	MachineZxPlus3(IMachineController*, Model);

	void insertDisk(cstr fpath, char side = 'A');
};
