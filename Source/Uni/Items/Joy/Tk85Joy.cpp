// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Tk85Joy.h"


namespace zxsp
{

#define i_addr nullptr // TODO


Tk85Joy::Tk85Joy(Machine* m) : Joy(m, isa_Tk85Joy, internal, nullptr, i_addr, "?")
{
	logline("TODO: Tk85Joy"); //
}


void Tk85Joy::input(Time /*t*/, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask)
{
	(void)addr;
	(void)byte;
	(void)mask;
	// TODO
}

} // namespace zxsp
