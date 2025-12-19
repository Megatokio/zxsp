// Copyright (c) 2006 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "KempstonJoy.h"


namespace zxsp
{

class InvesJoy final : public KempstonJoy
{
public:
	explicit InvesJoy(Machine*);

protected:
	~InvesJoy() override = default;
};

} // namespace zxsp
