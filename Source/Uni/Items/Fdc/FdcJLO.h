// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Fdc.h"


namespace zxsp
{

class FdcJLO final : public Fdc
{
public:
	explicit FdcJLO(Machine*);

protected:
	~FdcJLO() override = default;
};

} // namespace zxsp
