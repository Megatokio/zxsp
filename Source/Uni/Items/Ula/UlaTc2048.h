#pragma once
/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#include "UlaZxsp.h"


class UlaTc2048 : public UlaZxsp
{
	uint8	byte_ff;

public:
	UlaTc2048(Machine*, isa_id);

	void	powerOn					(/*t=0*/ int32 cc) override;
	void	reset					(Time t, int32 cc) override;
	void	output					(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void	input					(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	int32	doFrameFlyback			(int32 cc) override;
	int32	updateScreenUpToCycle	(int32 cc) override;
	//void	drawVideoBeamIndicator	(int32 cc) override;
	void	markVideoRam			() override;
	//int32	addWaitCycles			(int32 cc, uint16 addr) volatile const override;	TODO ?

	uint8	getPortFE()				{ return ula_out_byte; }
	bool	is64ColumnMode()		{ return byte_ff&4; }

	bool	hasPortFF				() volatile const noexcept override	{ return yes; }
	void	setPortFF				(uint8) override;
	uint8   getPortFF				()	volatile const override			{ return byte_ff; }
};
































