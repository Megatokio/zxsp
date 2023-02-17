#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

namespace gui
{

class PrinterAercoInsp : public Inspector
{
public:
	PrinterAercoInsp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
