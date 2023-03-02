#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuZx81.h"


class MmuTk85 : public MmuZx81
{
public:
	explicit MmuTk85(Machine*);

protected:
	~MmuTk85() override = default;
};
