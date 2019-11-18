/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#ifndef MMUTC2048_H
#define MMUTC2048_H

#include "MmuZxsp.h"


class MmuTc2048 : public MmuZxsp
{
protected:
	uint8	port_F4;

public:
	explicit MmuTc2048(Machine*);

VIR	void	selectEXROM 	(bool) {}

protected:
	MmuTc2048(Machine*, isa_id, cstr oaddr, cstr iaddr);

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;

	bool	hasPortF4()	volatile const noexcept override { return yes; }	 // see note on Basic64-Demo.tzx in *.cpp
	uint8	getPortF4()	volatile const override			 { return port_F4; } // seems to be present but
	void	setPortF4(uint8 n) override					 { port_F4 = n; }	 // seems to have no function
	//void	romCS(bool disable) override;
};


#endif


















