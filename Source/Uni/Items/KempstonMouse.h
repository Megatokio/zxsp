#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Mouse.h"
#include "globals.h"


class KempstonMouse : public Item
{
	int scale;
	int x;
	int y;

protected:
	~KempstonMouse() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd(Time t) override;
	// void	videoFrameEnd(int32 cc) override;

public:
	explicit KempstonMouse(Machine*);

	void  setScale(int n);
	int	  getScale() const volatile { return scale; }
	uint8 getXPos();
	uint8 getYPos();
	uint8 getButtons() const;
};
