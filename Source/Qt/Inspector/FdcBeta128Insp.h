#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcBeta128.h"
#include "Inspector.h"
#include <QObject>

namespace zxsp
{

class FdcBeta128Insp : public Inspector
{
public:
	FdcBeta128Insp(QWidget*, MachineController*, volatile FdcBeta128*);
};

} // namespace zxsp
