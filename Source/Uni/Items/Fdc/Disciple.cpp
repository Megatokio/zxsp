/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/


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
