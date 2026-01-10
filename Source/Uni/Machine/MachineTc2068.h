// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineTc2048.h"


namespace zxsp
{

class MachineTc2068 : public MachineTc2048
{
public:
	MachineTc2068(IMachineController*, IScreen*, Model);

	void insertCartridge(cstr fpath);
};

} // namespace zxsp
