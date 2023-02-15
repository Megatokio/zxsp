#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuZx80.h"


class MmuZx81 : public MmuZx80
{
public:
	explicit MmuZx81(Machine* m) : MmuZx80(m, isa_MmuZx81) {}

protected:
	MmuZx81(Machine* m, isa_id id) : MmuZx80(m, id) {}

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset			( Time t, int32 cc ) override;
	// void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	// void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	// void	audioBufferEnd	( Time t ) override;
	// void	videoFrameEnd	( int32 cc ) override;
	// void	saveToFile		( FD& ) const throws override;
	// void	loadFromFile	( FD& ) throws override;

	// Mmu interface:
	// bool	hasPort7ffd	() volatile const noexcept override	{ return no; }
	// bool	hasPort1ffd	() volatile const noexcept override	{ return no; }
	// bool	hasPortF4	() volatile const noexcept override	{ return no; }
	void romCS(bool disable) override; // called from read-side item ----> daisy chain
	// void	ramCS		(bool disable) override;  // called from read-side item ----> daisy chain
	//  currently not used. Ram extensions simply add to machine.ram.
};
