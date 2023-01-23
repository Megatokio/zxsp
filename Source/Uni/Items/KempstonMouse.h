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
	int	x;
	int	y;

public:
	explicit KempstonMouse(Machine*);
	~KempstonMouse();

	// Item interface:
	void	powerOn			( /*t=0*/ int32 cc ) override;
	//void	reset			( Time t, int32 cc ) override;
	void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	//void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	//void	audioBufferEnd	( Time t ) override;
	//void	videoFrameEnd	( int32 cc ) override;
	//void	saveToFile		( FD& fd ) const throws override;
	//void	loadFromFile	( FD& fd ) throws override;

	void	setScale(int n)				{ x = x/scale*n; y=y/scale*n; scale=n; }
	int		getScale() volatile const	{ return scale; }

	uint8 getXPos()
	{
		if(machine==front_machine) { int dx=mouse.dx; mouse.dx-=dx; x+=dx; }
		return x/scale;
	}

	uint8 getYPos()
	{
		if(machine==front_machine) { int dy=mouse.dy; mouse.dy-=dy; y+=dy; }
		return y/scale;
	}

	uint8 getButtons()
	{
		return mouse.isGrabbed() && machine==front_machine
			? 0xff - (mouse.getLeftButton()<<1) - mouse.getRightButton()	// 2-button version
			: 0xff;
	}
};




