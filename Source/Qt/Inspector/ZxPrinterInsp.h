// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"
#include "Printer/ZxPrinter.h"

namespace zxsp
{

class ZxPrinterInsp : public Inspector
{
public:
	ZxPrinterInsp(QWidget*, MachineController* mc, volatile ZxPrinter*);
};

} // namespace zxsp
