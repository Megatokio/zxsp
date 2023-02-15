#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

class ZonxBoxInsp : public Inspector
{
public:
	ZonxBoxInsp(QWidget*, MachineController* mc, volatile IsaObject*);
};
