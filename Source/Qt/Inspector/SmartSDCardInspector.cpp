// Copyright (c) 2015 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "SmartSDCardInspector.h"

namespace zxsp
{

SmartSDCardInspector::SmartSDCardInspector(QWidget* p, MachineController* m, volatile SmartSDCard* o) :
	Inspector(p, m, o)
{}

SmartSDCardInspector::~SmartSDCardInspector() {}

} // namespace zxsp
