#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface/Multiface128.h"
#include "MultifaceInsp.h"

namespace gui
{

class Multiface128Insp : public MultifaceInsp
{
	QLabel* label_visibility;

public:
	Multiface128Insp(QWidget*, MachineController*, volatile Multiface128*);

protected:
	void updateWidgets() override;
};

} // namespace gui
