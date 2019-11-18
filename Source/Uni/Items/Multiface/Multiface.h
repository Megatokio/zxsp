/*	Copyright  (c)	Günter Woigk 2015 - 2019
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

#ifndef MULTIFACE_H
#define MULTIFACE_H

#include "kio/kio.h"
#include "Item.h"
#include "Memory.h"


class Multiface : public Item
{
	friend class MultifaceInsp;

protected:
	MemoryPtr	rom;
	MemoryPtr	ram;
	bool		nmi_pending;
	bool		paged_in;

	void		page_in();
	void		page_out();

	Multiface(Machine*, isa_id, cstr rom, cstr o_addr, cstr i_addr);
	virtual ~Multiface();

protected:
	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;
	//uint8	handleRomPatch	(uint16 pc, uint8 o) override;		// returns new opcode
	//void	triggerNmi		() override;
	//uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	//void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};


#endif








