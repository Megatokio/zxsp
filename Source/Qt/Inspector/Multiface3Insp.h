#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface/Multiface3.h"
#include "MultifaceInsp.h"

namespace gui
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

} // namespace gui
