#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ay/FullerBox.h"
#include "Inspector.h"

namespace gui
{

class FullerBoxInsp : public Inspector
{
public:
	FullerBoxInsp(QWidget*, MachineController*, volatile FullerBox*);
};

} // namespace gui
