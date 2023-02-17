#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QComboBox;


namespace gui
{

class Memotech64kRamInsp : public Inspector
{
	QComboBox* jumper;

public:
	Memotech64kRamInsp(QWidget*, MachineController*, volatile IsaObject*);
};

} // namespace gui
