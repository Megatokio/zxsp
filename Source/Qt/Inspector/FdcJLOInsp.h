#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcJLO.h"
#include "Inspector.h"
#include <QObject>

namespace zxsp
{

class FdcJLOInsp : public Inspector
{
public:
	FdcJLOInsp(QWidget*, MachineController*, volatile FdcJLO*);
};

} // namespace zxsp
