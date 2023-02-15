// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "DidaktikMelodikInsp.h"
#include "Item.h"

DidaktikMelodikInsp::DidaktikMelodikInsp(QWidget* w, MachineController* mc, volatile IsaObject* i) :
	Inspector(w, mc, i, "/Images/didaktik_melodik.jpg")
{
	assert(i->isA(isa_DidaktikMelodik));
}
