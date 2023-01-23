#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ula128k.h"


class UlaPlus3 : public Ula128k
{
public:
	explicit UlaPlus3(Machine*);

	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	setPort7ffd		(uint8) override;
	int32	addWaitCycles	(int32 cc, uint16 addr) volatile const override;
	uint8	getFloatingBusByte(int32 cc) override;
};



