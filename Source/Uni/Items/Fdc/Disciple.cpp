// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Disciple.h"

/*	DISCiPLE Interface
	up to 2 Disk drives
	1 Centronics port
	2 Joystick ports (RH: Sinclair/Kempston, LH: Sinclair)
	2 ZX Network ports
*/


#define o_addr NULL	//TODO
#define i_addr NULL	//TODO


Disciple::Disciple(Machine*m)
:Fdc(m,isa_Disciple,external,o_addr,i_addr)
{
}
