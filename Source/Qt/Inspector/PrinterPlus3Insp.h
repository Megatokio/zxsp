#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Printer/PrinterPlus3.h"


namespace gui
{

class PrinterPlus3Insp : public Inspector
{
public:
	PrinterPlus3Insp(QWidget*, MachineController*, volatile PrinterPlus3*);
};

} // namespace gui
