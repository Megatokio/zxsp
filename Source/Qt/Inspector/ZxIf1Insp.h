#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

namespace gui
{

class ZxIf1Insp : public Inspector
{
public:
	ZxIf1Insp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
