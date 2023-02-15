#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaZxsp.h"


class UlaInves : public UlaZxsp
{
public:
	explicit UlaInves(Machine*);
	virtual ~UlaInves();

protected:
	// Item interface:
	// void	powerOn			(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	// void	saveToFile		(FD&) const throws override;
	// void	loadFromFile	(FD&) throws override;

	// Ula interface:
	int32 addWaitCycles(int32 cc, uint16 addr) const volatile override;
	uint8 getFloatingBusByte(int32 cc) override;
	void  setupTiming() override;
};
