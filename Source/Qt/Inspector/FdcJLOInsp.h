#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc/FdcJLO.h"
#include "Inspector.h"
#include <QObject>

namespace gui
{

class FdcJLOInsp : public Inspector
{
public:
	FdcJLOInsp(QWidget*, MachineController*, volatile FdcJLO*);
};

} // namespace gui
