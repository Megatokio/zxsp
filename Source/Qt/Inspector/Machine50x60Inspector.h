#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineInspector.h"
#include <QRadioButton>


class Machine50x60Inspector : public MachineInspector
{
public:
	Machine50x60Inspector(QWidget*, MachineController*, volatile Machine*);
};
