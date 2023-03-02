#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Printer/ZxPrinter.h"

namespace gui
{

class ZxPrinterInsp : public Inspector
{
public:
	ZxPrinterInsp(QWidget*, MachineController* mc, volatile ZxPrinter*);
};

} // namespace gui
