#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuZxsp.h"


class MmuTc2048 : public MmuZxsp
{
protected:
	uint8 port_F4;

public:
	explicit MmuTc2048(Machine*);

	virtual void selectEXROM(bool) {}

protected:
	MmuTc2048(Machine*, isa_id, cstr oaddr, cstr iaddr);
	~MmuTc2048() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	bool  hasPortF4() const volatile noexcept override { return yes; } // see note on Basic64-Demo.tzx in *.cpp
	uint8 getPortF4() const volatile override { return port_F4; }	   // seems to be present but
	void  setPortF4(uint8 n) override
	{
		port_F4 = n;
	} // seems to have no function
	  // void	romCS(bool disable) override;
};
