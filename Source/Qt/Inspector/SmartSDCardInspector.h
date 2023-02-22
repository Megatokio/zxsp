#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"

namespace gui
{

class SmartSDCardInspector : public Inspector
{
public:
	SmartSDCardInspector(QWidget* p, MachineController* m, volatile IsaObject* o);
	~SmartSDCardInspector() override;
};

} // namespace gui
