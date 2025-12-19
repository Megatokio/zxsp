// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "WafaDrive.h"

/*	Rotronics Wafadrive
	2 "Stringy" drives
	1 RS-232 port
	1 Centronics port
*/


namespace zxsp
{

#define o_addr nullptr // TODO
#define i_addr nullptr // TODO


WafaDrive::WafaDrive(Machine* m) : Item(m, isa_WafaDrive, isa_Item, external, o_addr, i_addr) {}

} // namespace zxsp
