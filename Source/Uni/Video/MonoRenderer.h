#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"
#include "graphics/gif/GifEncoder.h"


// ===========================================================
//					screen renderer:
// ===========================================================


class MonoRenderer : public Renderer
{
public:
	static constexpr int h_border	   = 64; // pixel, must be N*8
	static constexpr int v_border	   = 48; // pixel, must be N*8
	static constexpr int screen_width  = 256;
	static constexpr int screen_height = 192;
	static constexpr int width		   = screen_width + 2 * h_border;  // width of bits[]
	static constexpr int height		   = screen_height + 2 * v_border; // height of bits[]

	// pixel_per_cc = 2,
	// cc_screen	= screen_width/pixel_per_cc,	// 128 -> 256 pixel
	// cc_h_border  = h_border/pixel_per_cc			// 32  -> 64 pixel

	explicit MonoRenderer(isa_id id = isa_MonoRenderer) :
		Renderer(id, screen_width, screen_height, h_border, v_border, no /*!color*/)
	{}

	void drawScreen(
		uint8* new_pixels, uint q_screen_width, uint q_screen_height, uint q_width, uint q_height, uint q_h_border,
		uint q_v_border, uint32 cc_vbi);
};


// ===========================================================
//					gif file creation:
// ===========================================================


class MonoGifWriter : public GifWriter
{
public:
	static constexpr int h_border	  = 32; // pixel, must be N*8
	static constexpr int v_border	  = 24; // pixel, must be N*8
	static constexpr int screen_width = 256, screen_height = 192;
	static constexpr int width	= screen_width + 2 * h_border;	// width of bits[]
	static constexpr int height = screen_height + 2 * v_border; // height of bits[]

	static constexpr int pixel_per_cc = 2;
	static constexpr int cc_screen	  = screen_width / pixel_per_cc; // 128 -> 256 pixel
	static constexpr int cc_h_border  = h_border / pixel_per_cc;	 // 32  -> 64 pixel

protected:
	// MonoGifWriter(QObject* p, isa_id id, bool update_border, uint frames_per_second);

public:
	MonoGifWriter(bool update_border, uint frames_per_second);

	void writeFrame(
		uint8* new_pixels, uint screen_w, uint screen_h, uint frame_h, uint frame_w, uint screen_x0, uint screen_y0);
	void saveScreenshot(
		cstr path, uint8* new_pixels, uint screen_w, uint screen_h, uint frame_h, uint frame_w, uint screen_x0,
		uint screen_y0);
	void drawScreen(
		uint8* new_pixels, uint q_screen_width, uint q_screen_height, uint q_width, uint q_height, uint q_h_border,
		uint q_v_border);
};
