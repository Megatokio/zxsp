// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineZx81.h"


namespace zxsp
{

class MachineTs1500 : public MachineZx81
{
public:
	explicit MachineTs1500(IMachineController*, IScreen*);
};

} // namespace zxsp
