// Copyright (c) 2015 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Fdc/SmartSDCard.h"
#include "Inspector.h"

namespace zxsp
{

class SmartSDCardInspector : public Inspector
{
public:
	SmartSDCardInspector(QWidget* p, MachineController* m, volatile SmartSDCard* o);
	~SmartSDCardInspector() override;
};

} // namespace zxsp
