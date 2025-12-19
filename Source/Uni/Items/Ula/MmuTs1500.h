// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MmuZx81.h"


namespace zxsp
{

class MmuTs1500 : public MmuZx81
{
public:
	explicit MmuTs1500(Machine*);

protected:
	~MmuTs1500() override = default;
};

} // namespace zxsp
