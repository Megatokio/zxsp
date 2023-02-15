#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include <QObject>

class Tc2068JoyInsp : public JoyInsp
{
public:
	Tc2068JoyInsp(QWidget*, MachineController*, volatile IsaObject*, cstr img_path);

protected:
	void updateWidgets() override;
};
