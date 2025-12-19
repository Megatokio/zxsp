// Copyright (c) 2006 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy.h"


namespace zxsp
{

class KempstonJoy : public Joy
{
public:
	explicit KempstonJoy(
		Machine*, isa_id = isa_KempstonJoy, Internal = external,
		cstr i_addr = "----.----.000-.----" /*Kempston Issue 4*/);

protected:
	~KempstonJoy() override;

	// Item interface
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};

} // namespace zxsp
