/*	Copyright  (c)	Günter Woigk 2006 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef KEMPSTONMOUSE_H
#define KEMPSTONMOUSE_H

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


#endif

