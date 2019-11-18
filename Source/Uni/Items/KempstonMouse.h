#pragma once
/*	Copyright  (c)	Günter Woigk 2006 - 2019
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




