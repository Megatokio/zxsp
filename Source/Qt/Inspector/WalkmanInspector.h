#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeRecorderInsp.h"


namespace gui
{

class WalkmanInspector : public TapeRecorderInsp
{
public:
	WalkmanInspector(QWidget*, MachineController*, volatile Walkman*);

protected:
	void updateWidgets() override;
};

} // namespace gui
