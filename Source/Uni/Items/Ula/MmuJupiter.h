#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mmu.h"


class MmuJupiter : public Mmu
{
public:
	explicit MmuJupiter(Machine*);

protected:
	~MmuJupiter() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
};
