#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/DktronicsDualJoy.h"
#include "JoyInsp.h"


namespace gui
{

class DktronicsDualJoyInsp : public JoyInsp
{
public:
	DktronicsDualJoyInsp(QWidget*, MachineController*, volatile DktronicsDualJoy*);
};

} // namespace gui
