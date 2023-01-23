#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QObject>
#include <QLineEdit>
#include <QComboBox>
#include "JoyInsp.h"

class SinclairJoyInsp : public JoyInsp
{
public:
	SinclairJoyInsp( QWidget*, MachineController*, volatile IsaObject*, cstr img_path );

protected:
	void updateWidgets() override;
};




























