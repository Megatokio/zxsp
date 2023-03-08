#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"

class KempstonMouse : public Item
{
	int	   scale;
	int&   x;
	int&   y;
	uint8& buttons;

protected:
	~KempstonMouse() override = default;

	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;

public:
	explicit KempstonMouse(Machine*);

	void  setScale(int n);
	int	  getScale() const volatile { return scale; }
	uint8 getXPos() const volatile { return uint8(x / scale); }
	uint8 getYPos() const volatile { return uint8(y / scale); }
	uint8 getButtons() const volatile { return buttons & 3; }
};
