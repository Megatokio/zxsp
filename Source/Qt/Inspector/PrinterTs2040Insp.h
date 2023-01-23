#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

class PrinterTs2040Insp : public Inspector
{
public:
	PrinterTs2040Insp( QWidget*, MachineController*, volatile IsaObject* );
};


