// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "PrinterPlus3.h"



//    WoS:
//    Peripheral: +3 Centronics Interface.
//    Port: 0000 ---- ---- --0-

static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

PrinterPlus3::PrinterPlus3(Machine *parent) :
	Printer(parent,isa_PrinterPlus3,internal,o_addr,i_addr)
{
}
