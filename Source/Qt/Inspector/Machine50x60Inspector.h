#pragma once
// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineInspector.h"
#include <QRadioButton>


namespace zxsp
{
class Machine50x60Inspector : public MachineInspector
{
public:
	Machine50x60Inspector(QWidget*, MachineController*, volatile Machine*);
};
} // namespace zxsp
