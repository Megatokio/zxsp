// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Inspector.h"
#include "Ram/Memotech64kRam.h"
class QComboBox;


namespace zxsp
{

class Memotech64kRamInsp : public Inspector
{
	QComboBox* jumper;

public:
	Memotech64kRamInsp(QWidget*, MachineController*, volatile Memotech64kRam*);
};

} // namespace zxsp
