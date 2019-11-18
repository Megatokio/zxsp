/*	Copyright  (c)	Günter Woigk 2008 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#ifndef MMUZX81_H
#define MMUZX81_H

#include "MmuZx80.h"


class MmuZx81 : public MmuZx80
{
public:
	explicit MmuZx81(Machine* m)	: MmuZx80(m,isa_MmuZx81) {}

protected:
	MmuZx81(Machine* m, isa_id id)	: MmuZx80(m,id){}

// Item interface:
	void	powerOn			( /*t=0*/ int32 cc ) override;
	//void	reset			( Time t, int32 cc ) override;
	//void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	//void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	//void	audioBufferEnd	( Time t ) override;
	//void	videoFrameEnd	( int32 cc ) override;
	//void	saveToFile		( FD& ) const throws override;
	//void	loadFromFile	( FD& ) throws override;

// Mmu interface:
	//bool	hasPort7ffd	() volatile const noexcept override	{ return no; }
	//bool	hasPort1ffd	() volatile const noexcept override	{ return no; }
	//bool	hasPortF4	() volatile const noexcept override	{ return no; }
	void	romCS		(bool disable) override;  // called from read-side item ----> daisy chain
	//void	ramCS		(bool disable) override;  // called from read-side item ----> daisy chain
						// currently not used. Ram extensions simply add to machine.ram.
};


#endif









