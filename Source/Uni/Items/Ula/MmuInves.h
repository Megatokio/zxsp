// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MmuZxsp.h"


namespace zxsp
{

class MmuInves : public MmuZxsp
{
public:
	explicit MmuInves(Machine*);

protected:
	~MmuInves() override = default;
};

} // namespace zxsp
