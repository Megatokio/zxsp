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


#ifndef RENDERER_H
#define RENDERER_H


#include "IoInfo.h"
#include "IsaObject.h"
#include "gif/GifEncoder.h"


typedef uint32	RgbaColor;						// RGBA for OpenGL
extern const RgbaColor zxsp_rgba_colors[16];	// RGBA in ZxspRenderer.cpp

const RgbaColor black			= 0x000000FF;
const RgbaColor blue			= 0x0000CCFF;
const RgbaColor red				= 0xCC0000FF;
const RgbaColor magenta			= 0xCC00CCFF;
const RgbaColor green			= 0x00CC00FF;
const RgbaColor cyan			= 0x00CCCCFF;
const RgbaColor yellow			= 0xCCCC00FF;
const RgbaColor white			= 0xCCCCCCFF;
const RgbaColor bright_black	= 0x000000FF;
const RgbaColor bright_blue		= 0x0000FFFF;
const RgbaColor bright_red		= 0xFF0000FF;
const RgbaColor bright_magenta	= 0xFF00FFFF;
const RgbaColor bright_green	= 0x00FF00FF;
const RgbaColor bright_cyan		= 0x00FFFFFF;
const RgbaColor bright_yellow	= 0xFFFF00FF;
const RgbaColor bright_white	= 0xFFFFFFFF;
const RgbaColor grey			= 0x808080FF;


// ===========================================================
//					screen renderer:
// ===========================================================


class Renderer : public IsaObject
{
protected:
	Renderer(QObject* p, isa_id id, uint screen_width, uint screen_height, uint h_border, uint v_border, bool color);

public:
	uint		screen_width;		// = 256
	uint		screen_height;		// = 192
	uint		h_border;			// = 64,				// pixel, must be N*8
	uint		v_border;			// = 48,				// pixel, must be N*8
	uint		width;				// total width of bits[]
	uint		height;				// total height of bits[]

	union{ RgbaColor* bits; uint8* mono_octets; };

	~Renderer()						{ delete[] bits; }
};


// ===========================================================
//					gif file creation:
// ===========================================================


/*	Base class for Gif Writer.
	Sub classes must provide drawScreen(…), writeFrame(…) and saveScreenShot(…).
*/
class GifWriter : public IsaObject
{
protected:							// values for 32 column mode:
	const uint	screen_width;		// = 256
	const uint	screen_height;		// = 192
	const uint	h_border;			// = 32		// pixel, must be N*8
	const uint	v_border;			// = 24		// pixel
	const uint	width;				// = total width of bits[]
	const uint	height;				// = total height of bits[]

	uint		frame_count;		// for bits2
	Pixelmap*	bits;				// new screen
	Pixelmap*	diff;				// provided for diff
	Pixelmap*	bits2;				// old screen, not yet written to file
	Pixelmap*	diff2;				// old screen, not yet written to file, diff image to what is already in file
	bool		update_border;		// auch mit border animation?
	uint		frames_per_second;	// animation speed
	uint		frames_per_flashphase;	// for screenshot

	cColormap&	global_colormap;
	GifEncoder	gif_encoder;

	void		write_diff2_to_file() throws;

	GifWriter( QObject* p, isa_id id, cColormap&, uint screen_width, uint screen_height,
			   uint h_border, uint v_border, bool update_border, uint frames_per_second );

public:
	void		startRecording(cstr path) throws;
	void		stopRecording() throws;
};


#endif // RENDERER_H



















