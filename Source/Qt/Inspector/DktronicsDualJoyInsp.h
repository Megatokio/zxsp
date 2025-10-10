#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/DktronicsDualJoy.h"
#include "JoyInsp.h"


namespace gui
{

class DktronicsDualJoyInsp final : public JoyInsp
{
	volatile DktronicsDualJoy* const dkjoy;

public:
	DktronicsDualJoyInsp(QWidget*, MachineController*, volatile DktronicsDualJoy*);

protected:
	//void updateWidgets() override;
	cstr lineedit_text(uint port, uint8 state) override;
};

} // namespace gui
