#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Printer/PrinterLprint3.h"


namespace zxsp
{

class PrinterLprint3Insp : public Inspector
{
public:
	PrinterLprint3Insp(QWidget*, MachineController*, volatile PrinterLprint3*);
};

} // namespace zxsp
