// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "OpusDiscovery.h"

/*	Opus Discovery
	up to 2 Disk drives
	1 Centronics port
	1 Joystick port (Kempston)
*/

#define o_addr NULL	//TODO
#define i_addr NULL	//TODO


OpusDiscovery::OpusDiscovery(Machine*m)
:Fdc(m,isa_OpusDiscovery,external,o_addr,i_addr)
{
}




