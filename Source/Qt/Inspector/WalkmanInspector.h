// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "TapeRecorderInsp.h"


namespace zxsp
{

class WalkmanInspector : public TapeRecorderInsp
{
public:
	WalkmanInspector(QWidget*, MachineController*, volatile Walkman*);

protected:
	void updateWidgets() override;
};

} // namespace zxsp
