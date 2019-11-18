#pragma once
/*	Copyright  (c)	Günter Woigk 2013 - 2019
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

#include "ZxspRenderer.h"


// ===========================================================
//					screen renderer:
// ===========================================================


class Tc2048Renderer : public ZxspRenderer
{
public:
	static const int
		h_border	 = 64*2,			// pixel, must be N*8
		v_border	 = 48,				// pixel, must be N*8
		screen_width = 256*2,
		screen_height= 192,
		width		 = screen_width+2*h_border,		// width of bits[]
		height		 = screen_height+2*v_border,	// height of bits[]

		pixel_per_cc = 2*2,

		cc_screen	 = screen_width/pixel_per_cc,	// 128 -> 256 pixel
		cc_h_border	 = h_border/pixel_per_cc;		// 32  -> 64 pixel


	explicit Tc2048Renderer(QObject*p)
					:ZxspRenderer(p,isa_Tc2048Renderer,screen_width,screen_height,h_border,v_border){}

	void drawScreen( IoInfo*, uint ioinfo_count, uint8 *attr_pixels, uint cc_per_scanline,
					 uint32 cc_start_of_screenfile, bool flashphase, uint32 cc_vbi) override;
};


// ===========================================================
//					gif file creation:
// ===========================================================


class Tc2048GifWriter : public ZxspGifWriter
{
public:
	Tc2048GifWriter( QObject* p, bool update_border );

	void drawScreen( IoInfo*, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline,
					 uint32 cc_start_of_screenfile, bool flashphase) override;
};























