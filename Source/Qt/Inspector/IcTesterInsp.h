#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

namespace gui
{

class IcTesterInsp : public Inspector
{
public:
	IcTesterInsp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
