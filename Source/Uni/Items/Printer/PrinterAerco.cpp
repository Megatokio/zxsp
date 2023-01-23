// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "PrinterAerco.h"


//    Wos:
//    Peripheral: Aerco Centronics Interface.
//    Port: ---- ---- 0111 1111
//	  Port # 0111 1111 RW

static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO


PrinterAerco::PrinterAerco(Machine*m) :
	Printer(m,isa_PrinterAerco,external,o_addr,i_addr)
{
}
