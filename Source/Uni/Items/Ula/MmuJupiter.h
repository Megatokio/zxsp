// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Mmu.h"


namespace zxsp
{

class MmuJupiter : public Mmu
{
public:
	explicit MmuJupiter(Machine*);

protected:
	~MmuJupiter() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
};

} // namespace zxsp
