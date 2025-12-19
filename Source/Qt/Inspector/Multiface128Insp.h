// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Multiface/Multiface128.h"
#include "MultifaceInsp.h"

namespace zxsp
{

class Multiface128Insp : public MultifaceInsp
{
	QLabel* label_visibility;

public:
	Multiface128Insp(QWidget*, MachineController*, volatile Multiface128*);

protected:
	void updateWidgets() override;
};

} // namespace zxsp
