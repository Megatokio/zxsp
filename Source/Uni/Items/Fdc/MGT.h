// Copyright (c) 2023 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Fdc.h"


namespace zxsp
{

class MGT final : public Fdc
{
public:
	explicit MGT(Machine*);

protected:
	~MGT() override = default;
};

} // namespace zxsp
