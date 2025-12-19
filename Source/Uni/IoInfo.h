// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Templates/Array.h"


namespace zxsp
{

struct IoInfo
{
	int32  cc;
	uint16 addr;
	uint8  byte;
	uint8  mask;

	IoInfo() {}
	IoInfo(int32 cc, uint16 addr, uint8 byte, uint8 mask = 0xff) : cc(cc), addr(addr), byte(byte), mask(mask) {}
};


using IoArray = Array<IoInfo>;

} // namespace zxsp
