#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Memory.h"
#include "kio/kio.h"


class Multiface : public Item
{
	friend class MultifaceInsp;

protected:
	MemoryPtr rom;
	MemoryPtr ram;
	bool	  nmi_pending;
	bool	  paged_in;

	void page_in();
	void page_out();

	Multiface(Machine*, isa_id, cstr rom, cstr o_addr, cstr i_addr);
	virtual ~Multiface();

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	// uint8	handleRomPatch	(uint16 pc, uint8 o) override;		// returns new opcode
	// void	triggerNmi		() override;
	// uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	// void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};
