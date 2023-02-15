#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxspRenderer.h"


// ===========================================================
//					screen renderer:
// ===========================================================


class Tc2048Renderer : public ZxspRenderer
{
public:
	static const int h_border = 64 * 2, // pixel, must be N*8
		v_border			  = 48,		// pixel, must be N*8
		screen_width = 256 * 2, screen_height = 192,
					 width = screen_width + 2 * h_border,  // width of bits[]
		height			   = screen_height + 2 * v_border, // height of bits[]

		pixel_per_cc = 2 * 2,

					 cc_screen = screen_width / pixel_per_cc, // 128 -> 256 pixel
		cc_h_border			   = h_border / pixel_per_cc;	  // 32  -> 64 pixel


	explicit Tc2048Renderer(QObject* p) :
		ZxspRenderer(p, isa_Tc2048Renderer, screen_width, screen_height, h_border, v_border)
	{}

	void drawScreen(
		IoInfo*,
		uint   ioinfo_count,
		uint8* attr_pixels,
		uint   cc_per_scanline,
		uint32 cc_start_of_screenfile,
		bool   flashphase,
		uint32 cc_vbi) override;
};


// ===========================================================
//					gif file creation:
// ===========================================================


class Tc2048GifWriter : public ZxspGifWriter
{
public:
	Tc2048GifWriter(QObject* p, bool update_border);

	void drawScreen(
		IoInfo*,
		uint   ioinfo_count,
		uint8* attr_pixels,
		uint   cc_per_scanline,
		uint32 cc_start_of_screenfile,
		bool   flashphase) override;
};
