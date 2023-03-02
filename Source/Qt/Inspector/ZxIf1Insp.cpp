// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxIf1Insp.h"

namespace gui
{

ZxIf1Insp::ZxIf1Insp(QWidget* w, MachineController* mc, volatile ZxIf1* i) :
	Inspector(w, mc, i, "/Backgrounds/light-150-s.jpg")
{}

} // namespace gui
