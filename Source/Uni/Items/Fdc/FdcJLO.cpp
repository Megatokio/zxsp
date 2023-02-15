// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "FdcJLO.h"


//    WoS:

//    Peripheral: JLO (Status/Command).
//    Port: ---- ---- 1000 1111

//    Peripheral: JLO (Track).
//    Port: ---- ---- 1001 1111

//    Peripheral: JLO (Sector).
//    Port: ---- ---- 1010 1111

//    Peripheral: JLO (Data).
//    Port: ---- ---- 1011 1111

//    Peripheral: JLO (Select).
//    Port: ---- ---- 1011 0111

/*	1.  Port # 1000 1111, RW, read status/write command reg
	2.  Port # 1001 1111, RW, read/write track reg
	3.  Port # 1010 1111, RW, read/write sector reg
	4.  Port # 1011 1111, RW, read/write data reg
	5.  Port # 1011 0111,  W, drive/density/side select
*/

static cstr o_addr = nullptr; // TODO
static cstr i_addr = nullptr; // TODO

FdcJLO::FdcJLO(Machine* m) : Fdc(m, isa_FdcJLO, external, o_addr, i_addr) {}
