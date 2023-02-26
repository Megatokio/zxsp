// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "SmartSDCardInspector.h"

namespace gui
{

SmartSDCardInspector::SmartSDCardInspector(QWidget* p, MachineController* m, volatile SmartSDCard* o) :
	Inspector(p, m, o)
{}

SmartSDCardInspector::~SmartSDCardInspector() {}

} // namespace gui
