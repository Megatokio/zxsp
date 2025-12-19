#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Printer/PrinterAerco.h"


namespace zxsp
{

class PrinterAercoInsp : public Inspector
{
public:
	PrinterAercoInsp(QWidget*, MachineController*, volatile PrinterAerco*);
};

} // namespace zxsp
