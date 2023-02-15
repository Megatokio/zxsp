#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeRecorderInsp.h"


class WalkmanInspector : public TapeRecorderInsp
{
public:
	WalkmanInspector(QWidget*, MachineController*, volatile IsaObject*);

protected:
	void updateWidgets() override;
};
