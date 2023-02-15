#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


class KempstonJoy : public Joy
{
public:
	explicit KempstonJoy(
		Machine*,
		isa_id		= isa_KempstonJoy,
		Internal	= external,
		cstr i_addr = "----.----.000-.----" /*Kempston Issue 4*/);
	virtual ~KempstonJoy();

protected:
	// Item interface
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};
