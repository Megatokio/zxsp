#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include <QObject>
#include "JoyInsp.h"

class KempstonJoyInsp : public JoyInsp
{
public:
	KempstonJoyInsp( QWidget*, MachineController*, volatile IsaObject* );
};




























