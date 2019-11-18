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
#ifndef ULAINVES_H
#define ULAINVES_H

#include "UlaZxsp.h"


class UlaInves : public UlaZxsp
{
public:
	explicit UlaInves(Machine*);
	virtual ~UlaInves();

protected:
// Item interface:
	//void	powerOn			(/*t=0*/ int32 cc) override;
	//void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;

// Ula interface:
	int32	addWaitCycles(int32 cc, uint16 addr) volatile const override;
	uint8	getFloatingBusByte(int32 cc) override;
	void	setupTiming() override;
};


#endif









