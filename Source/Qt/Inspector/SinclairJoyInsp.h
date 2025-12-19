// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "JoyInsp.h"
#include <QComboBox>
#include <QLineEdit>
#include <QObject>

namespace zxsp
{

class SinclairJoyInsp : public JoyInsp
{
public:
	SinclairJoyInsp(QWidget*, MachineController*, volatile SinclairJoy*, cstr img_path);

protected:
	cstr lineedit_text(uint port, uint8 state) override;
};

} // namespace zxsp
