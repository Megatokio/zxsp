#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include <QObject>


namespace gui
{

class FdcD80Insp : public Inspector
{
public:
	FdcD80Insp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
