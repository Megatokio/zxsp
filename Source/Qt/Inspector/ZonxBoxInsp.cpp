// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZonxBoxInsp.h"
#include "Ay/Ay.h"


ZonxBoxInsp::ZonxBoxInsp(QWidget* w, MachineController* mc, volatile IsaObject* i) :
	Inspector(w, mc, i, "/Images/zonx.jpg")
{
	assert(i->isA(isa_ZonxBox) || i->isA(isa_ZonxBox81));
}
