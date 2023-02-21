#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include <QObject>

namespace gui
{

class KempstonJoyInsp : public JoyInsp
{
public:
	KempstonJoyInsp(QWidget*, MachineController*, volatile KempstonJoy*);
};

} // namespace gui
