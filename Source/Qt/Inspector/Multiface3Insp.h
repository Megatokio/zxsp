// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Multiface/Multiface3.h"
#include "MultifaceInsp.h"

namespace zxsp
{

class Multiface3Insp : public MultifaceInsp
{
	QLabel* label_visible;
	QLabel* label_ramonly;

public:
	Multiface3Insp(QWidget*, MachineController*, volatile Multiface3*);

protected:
	void updateWidgets() override;
};

} // namespace zxsp
