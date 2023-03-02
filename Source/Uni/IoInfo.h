#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Templates/Array.h"


struct IoInfo
{
	uint32 cc;
	uint16 addr;
	uint8  byte;
	uint8  mask;

	IoInfo() {}
	IoInfo(uint32 cc, uint16 addr, uint8 byte, uint8 mask = 0xff) : cc(cc), addr(addr), byte(byte), mask(mask) {}
};


using IoArray = Array<IoInfo>;
