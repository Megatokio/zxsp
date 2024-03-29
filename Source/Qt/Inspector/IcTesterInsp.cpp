// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IcTesterInsp.h"
#include "IcTester.h"


namespace gui
{

IcTesterInsp::IcTesterInsp(QWidget* w, MachineController* mc, volatile IcTester* i) :
	Inspector(w, mc, i, "/Backgrounds/light-150-s.jpg")
{
	assert(i->isA(isa_IcTester));
}

} // namespace gui
