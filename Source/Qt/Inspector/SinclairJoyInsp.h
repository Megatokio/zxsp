#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include <QComboBox>
#include <QLineEdit>
#include <QObject>

namespace gui
{

class SinclairJoyInsp : public JoyInsp
{
public:
	SinclairJoyInsp(QWidget*, MachineController*, volatile IsaObject*, cstr img_path);

protected:
	void updateWidgets() override;
};

} // namespace gui
