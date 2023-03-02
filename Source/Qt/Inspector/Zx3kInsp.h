#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
#include "Ram/Zx3kRam.h"
class QRadioButton;

namespace gui
{

class Zx3kInsp : public Inspector
{
	volatile Zx3kRam* const zx3kram;

	QRadioButton* button1k;
	QRadioButton* button2k;
	QRadioButton* button3k;

public:
	Zx3kInsp(QWidget*, MachineController*, volatile Zx3kRam*);

private:
	void slotSetRamSize(uint);
};

} // namespace gui
