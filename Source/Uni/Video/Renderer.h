#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IoInfo.h"
#include "IsaObject.h"
#include "graphics/gif/GifEncoder.h"


using RgbaColor = uint32;					 // RGBA for OpenGL
extern const RgbaColor zxsp_rgba_colors[16]; // RGBA in ZxspRenderer.cpp

constexpr RgbaColor black		   = 0x000000FF;
constexpr RgbaColor blue		   = 0x0000CCFF;
constexpr RgbaColor red			   = 0xCC0000FF;
constexpr RgbaColor magenta		   = 0xCC00CCFF;
constexpr RgbaColor green		   = 0x00CC00FF;
constexpr RgbaColor cyan		   = 0x00CCCCFF;
constexpr RgbaColor yellow		   = 0xCCCC00FF;
constexpr RgbaColor white		   = 0xCCCCCCFF;
constexpr RgbaColor bright_black   = 0x000000FF;
constexpr RgbaColor bright_blue	   = 0x0000FFFF;
constexpr RgbaColor bright_red	   = 0xFF0000FF;
constexpr RgbaColor bright_magenta = 0xFF00FFFF;
constexpr RgbaColor bright_green   = 0x00FF00FF;
constexpr RgbaColor bright_cyan	   = 0x00FFFFFF;
constexpr RgbaColor bright_yellow  = 0xFFFF00FF;
constexpr RgbaColor bright_white   = 0xFFFFFFFF;
constexpr RgbaColor grey		   = 0x808080FF;


// ===========================================================
//					screen renderer:
// ===========================================================


class Renderer : public IsaObject
{
protected:
	Renderer(QObject* p, isa_id id, uint screen_width, uint screen_height, uint h_border, uint v_border, bool color);

public:
	uint screen_width;	// = 256
	uint screen_height; // = 192
	uint h_border;		// = 64,				// pixel, must be N*8
	uint v_border;		// = 48,				// pixel, must be N*8
	uint width;			// total width of bits[]
	uint height;		// total height of bits[]

	union
	{
		RgbaColor* bits;
		uint8*	   mono_octets;
	};

	~Renderer() override { delete[] bits; }
};


// ===========================================================
//					gif file creation:
// ===========================================================


/*	Base class for Gif Writer.
	Sub classes must provide drawScreen(…), writeFrame(…) and saveScreenShot(…).
*/
class GifWriter : public IsaObject
{
protected:					  // values for 32 column mode:
	const uint screen_width;  // = 256
	const uint screen_height; // = 192
	const uint h_border;	  // = 32		// pixel, must be N*8
	const uint v_border;	  // = 24		// pixel
	const uint width;		  // = total width of bits[]
	const uint height;		  // = total height of bits[]

	uint	  frame_count;			 // for bits2
	Pixelmap* bits;					 // new screen
	Pixelmap* diff;					 // provided for diff
	Pixelmap* bits2;				 // old screen, not yet written to file
	Pixelmap* diff2;				 // old screen, not yet written to file, diff image to what is already in file
	bool	  update_border;		 // auch mit border animation?
	uint	  frames_per_second;	 // animation speed
	uint	  frames_per_flashphase; // for screenshot

	const Colormap& global_colormap;
	GifEncoder		gif_encoder;

	void write_diff2_to_file();

	GifWriter(
		QObject* p, isa_id id, const Colormap&, uint screen_width, uint screen_height, uint h_border, uint v_border,
		bool update_border, uint frames_per_second);

public:
	void startRecording(cstr path);
	void stopRecording();
};
