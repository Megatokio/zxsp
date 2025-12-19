// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZonxBoxInsp.h"
#include "Ay/Ay.h"


namespace zxsp
{

ZonxBoxInsp::ZonxBoxInsp(QWidget* w, MachineController* mc, volatile Ay* i) : Inspector(w, mc, i, "/Images/zonx.jpg")
{
	assert(i->isA(isa_ZonxBox) || i->isA(isa_ZonxBox81));
}

} // namespace zxsp
