// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy.h"


namespace zxsp
{

class DktronicsDualJoy final : public Joy
{
public:
	explicit DktronicsDualJoy(Machine*);

protected:
	~DktronicsDualJoy() override = default;

	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};

} // namespace zxsp
