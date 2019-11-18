#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "Mmu.h"


class Mmu128k : public Mmu
{
protected:
	uint8	port_7ffd;

public:
	explicit Mmu128k(Machine*);

VIR void	setMmuLocked	(bool f)					{ setPort7ffd( f ? port_7ffd|0x20 : port_7ffd&~0x20 ); }
VIR uint    getPageC000		() volatile const noexcept	{ return port_7ffd&7; }
VIR uint    getPage8000		() volatile const noexcept	{ return 2; }
VIR uint    getPage4000		() volatile const noexcept	{ return 5; }
VIR uint    getPage0000		() volatile const noexcept	{ return (port_7ffd>>4) & 1; }	// except if ROMDIS
	uint    getVideopage	() volatile const noexcept	{ return port_7ffd&8 ? 7 : 5; }
	bool	port7ffdIsLocked() volatile const noexcept	{ return port_7ffd&0x20; }

protected:
	Mmu128k(Machine*, isa_id, cstr o_addr, cstr i_addr);

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	// Mmu interface:
	void    romCS(bool active) override;	// from rear-item: daisy chain
	bool	hasPort7ffd() volatile const noexcept override	{ return yes; }
	uint8	getPort7ffd() volatile const override			{ return port_7ffd; }
	void	setPort7ffd(uint8) override;

private:
	void	page_rom_128k();
	void	page_ram_128k();
	void	page_mem_128k();
};












