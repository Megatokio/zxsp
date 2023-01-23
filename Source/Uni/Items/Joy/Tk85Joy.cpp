// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Tk85Joy.h"


#define i_addr		NULL		// TODO


Tk85Joy::Tk85Joy(Machine*m)
: Joy(m,isa_Tk85Joy,internal,NULL,i_addr,"?")
{
}



void Tk85Joy::input( Time /*t*/, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask )
{
	(void)addr;(void)byte;(void)mask;
	//TODO
}
