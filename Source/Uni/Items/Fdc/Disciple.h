// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Fdc.h"


namespace zxsp
{

class Disciple final : public Fdc
{
public:
	explicit Disciple(Machine*);

protected:
	~Disciple() override = default;
};

} // namespace zxsp
