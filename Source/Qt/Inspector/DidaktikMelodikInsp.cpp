// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "DidaktikMelodikInsp.h"

namespace gui
{

DidaktikMelodikInsp::DidaktikMelodikInsp(QWidget* w, MachineController* mc, volatile DidaktikMelodik* i) :
	Inspector(w, mc, i, "/Images/didaktik_melodik.jpg")
{}

} // namespace gui
