#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Printer/PrinterTs2040.h"


namespace gui
{

class PrinterTs2040Insp : public Inspector
{
public:
	PrinterTs2040Insp(QWidget*, MachineController*, volatile PrinterTs2040*);
};

} // namespace gui
