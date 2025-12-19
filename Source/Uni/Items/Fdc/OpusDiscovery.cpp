// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "OpusDiscovery.h"

/*	Opus Discovery
	up to 2 Disk drives
	1 Centronics port
	1 Joystick port (Kempston)
*/

namespace zxsp
{

#define o_addr nullptr // TODO
#define i_addr nullptr // TODO


OpusDiscovery::OpusDiscovery(Machine* m) : Fdc(m, isa_OpusDiscovery, external, o_addr, i_addr) {}

} // namespace zxsp
