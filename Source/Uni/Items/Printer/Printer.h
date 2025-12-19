// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"


namespace zxsp
{

class Printer : public Item
{
protected:
	Printer(Machine*, isa_id, Internal internal, cstr o_addr, cstr i_addr);

protected:
	~Printer() override = default;

	// Item interface:
	// void	powerOn			( /*t=0*/ int32 cc ) override;
	// void	reset			( Time t, int32 cc ) override;
	// void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	// void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	// void	audioBufferEnd	( Time t ) override;
	// void	videoFrameEnd	( int32 cc ) override;
};

} // namespace zxsp
