#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


class DktronicsDualJoy : public Joy
{
public:
	explicit DktronicsDualJoy(Machine*);

protected:
	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};



















