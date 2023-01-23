// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "AmxMouse.h"


/*	-->  http://velesoft.speccy.cz/othermouse-cz.htm
	Circuit also on k1
*/

#define o_addr NULL	//TODO
#define i_addr NULL	//TODO


AmxMouse::AmxMouse(Machine*m)
: Item(m,isa_AmxMouse,isa_Mouse,external,o_addr,i_addr)
{
}
