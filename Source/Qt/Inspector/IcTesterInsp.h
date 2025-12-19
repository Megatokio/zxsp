#pragma once
// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IcTester.h"
#include "Inspector.h"


namespace zxsp
{

class IcTesterInsp : public Inspector
{
public:
	IcTesterInsp(QWidget*, MachineController*, volatile IcTester*);
};

} // namespace zxsp
