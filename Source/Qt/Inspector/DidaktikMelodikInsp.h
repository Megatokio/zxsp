// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Ay/AySubclasses.h"
#include "AyInsp.h"

namespace zxsp
{

class DidaktikMelodikInsp : public AyInsp
{
public:
	DidaktikMelodikInsp(QWidget*, MachineController*, volatile DidaktikMelodik*);
};

} // namespace zxsp
