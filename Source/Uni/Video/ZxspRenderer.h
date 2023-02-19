#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"
#include "graphics/gif/GifEncoder.h"


// ===========================================================
//					screen renderer:
// ===========================================================


class ZxspRenderer : public Renderer
{
protected:
	ZxspRenderer(QObject* p, isa_id id, uint sw, uint sh, uint bw, uint bh) :
		Renderer(p, id, sw, sh, bw, bh, yes /*color*/)
	{}

public:
	static constexpr int h_border	   = 64; // pixel, must be N*8
	static constexpr int v_border	   = 48; // pixel, must be N*8
	static constexpr int screen_width  = 256;
	static constexpr int screen_height = 192;
	static constexpr int width		   = screen_width + 2 * h_border;  // width of bits[]
	static constexpr int height		   = screen_height + 2 * v_border; // height of bits[]

	static constexpr int pixel_per_cc = 2;
	static constexpr int cc_screen	  = screen_width / pixel_per_cc; // 128 -> 256 pixel
	static constexpr int cc_h_border  = h_border / pixel_per_cc;	 // 32  -> 64 pixel


	explicit ZxspRenderer(QObject* p) :
		Renderer(p, isa_ZxspRenderer, screen_width, screen_height, h_border, v_border, yes /*color*/)
	{}

	virtual void drawScreen(
		IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
		bool flashphase, uint32 cc);
};


// ===========================================================
//					gif file creation:
// ===========================================================


class ZxspGifWriter : public GifWriter
{
public:
	static constexpr int h_border	   = 32; // pixel, must be N*8
	static constexpr int v_border	   = 24; // pixel, must be N*8
	static constexpr int screen_width  = 256;
	static constexpr int screen_height = 192;
	static constexpr int width		   = screen_width + 2 * h_border;  // width of bits[]
	static constexpr int height		   = screen_height + 2 * v_border; // height of bits[]

	static constexpr int pixel_per_cc = 2;
	static constexpr int cc_screen	  = screen_width / pixel_per_cc; // 128 -> 256 pixel
	static constexpr int cc_h_border  = h_border / pixel_per_cc;	 // 32  -> 64 pixel

protected:
	ZxspGifWriter(QObject* p, isa_id id, const Colormap&, bool update_border, uint frames_per_second);

public:
	ZxspGifWriter(QObject* p, bool update_border, uint frames_per_second = 50);

	virtual void drawScreen(
		IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
		bool flashphase);

	void writeFrame(
		IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
		bool flashphase);

	void saveScreenshot(
		cstr path, IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline,
		uint32 cc_start_of_screenfile);
};
