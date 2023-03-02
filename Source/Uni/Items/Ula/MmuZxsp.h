#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mmu.h"


class MmuZxsp : public Mmu
{
public:
	explicit MmuZxsp(Machine*);

protected:
	MmuZxsp(Machine*, isa_id, cstr o_addr, cstr i_addr);
	~MmuZxsp() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	// Mmu interface:
	void mapMem() override;
	void romCS(bool disable) override; // called from rear-side rom ---> daisy chain
};
