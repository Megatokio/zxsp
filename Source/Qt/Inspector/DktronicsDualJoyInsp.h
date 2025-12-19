// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy/DktronicsDualJoy.h"
#include "JoyInsp.h"


namespace zxsp
{

class DktronicsDualJoyInsp final : public JoyInsp
{
public:
	DktronicsDualJoyInsp(QWidget*, MachineController*, volatile DktronicsDualJoy*);

protected:
	//void updateWidgets() override;
	cstr lineedit_text(uint port, uint8 state) override;
};

} // namespace zxsp
