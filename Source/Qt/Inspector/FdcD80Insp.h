#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcD80.h"
#include "Inspector.h"
#include <QObject>

namespace zxsp
{

class FdcD80Insp : public Inspector
{
public:
	FdcD80Insp(QWidget*, MachineController*, volatile FdcD80*);
};

} // namespace zxsp
