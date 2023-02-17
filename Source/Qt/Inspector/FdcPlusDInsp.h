#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include <QObject>


namespace gui
{

class FdcPlusDInsp : public Inspector
{
public:
	FdcPlusDInsp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
