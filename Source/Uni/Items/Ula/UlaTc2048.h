#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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
































