// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MmuZx81.h"


namespace zxsp
{

class MmuTk85 : public MmuZx81
{
public:
	explicit MmuTk85(Machine*);

protected:
	~MmuTk85() override = default;
};

} // namespace zxsp
