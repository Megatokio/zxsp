// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "FullerBoxInsp.h"

namespace gui
{

FullerBoxInsp::FullerBoxInsp(QWidget* w, MachineController* mc, volatile IsaObject* i) :
	Inspector(w, mc, i, "/Backgrounds/light-150-s.jpg")
{}

} // namespace gui
