// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"

namespace zxsp
{

constexpr RgbaColor zx80_rgba_colors[16] = { // RGBA
	black,		  blue,		   red,		   magenta,		   green,		 cyan,		  yellow,		 white,
	bright_black, bright_blue, bright_red, bright_magenta, bright_green, bright_cyan, bright_yellow, bright_white};


/* global ZX Spectrum color table:
		8 x normal brightness
		8 x bright
*/
using GifColor						  = uint8;
static constexpr GifColor transp	  = 8;										// 'bright black' used for transparency
static constexpr Comp	  F			  = (bright_white >> 8) & 0xff;				// "bright": full brightness
static constexpr Comp	  H			  = (white >> 8) & 0xff;					// "normal": reduced brightness: 80%
static constexpr Comp	  zx_colors[] = {										//
	0, 0, 0, 0, 0, H, H, 0, 0, H, 0, H, 0, H, 0, 0, H, H, H, H, 0, H, H, H, // r,g,b
	0, 0, 0, 0, 0, F, F, 0, 0, F, 0, F, 0, F, 0, 0, F, F, F, F, 0, F, F, F};
static Colormap			  zxsp_colormap(zx_colors, 16, transp);


template<typename Color>
inline constexpr Color color(int index)
{
	return zx80_rgba_colors[index];
}
template<>
inline constexpr uint8 color<uint8>(int index)
{
	return index;
}


template<typename Color>
void zx80Renderer(VideoFrame<Color>* videoframe, VideoData* newframedata)
{
	// render b&w video image in newframedata into the output videoframe.
	// the input frame is cropped to fit in the limits of the videoframe.
	// the final frame size and screen rect are stored into the videoframe.
	// the pixels from the input frame are rendered into the videoframe.

	assert(newframedata->what == VideoData::Zx80Frame);
	Zx80VideoData* newdata = reinterpret_cast<Zx80VideoData*>(newframedata);

	{
		static constexpr bool is_ic			= sizeof(Color) == 1;
		static constexpr int  screen_width	= 256;
		static constexpr int  screen_height = 192;
		static constexpr int  h_border		= is_ic ? 32 : 6 * 8;			// pixels, must be N*8
		static constexpr int  v_border		= is_ic ? 24 : 6 * 6;			// pixels
		static constexpr int  width			= screen_width + 2 * h_border;	// width of bits[]
		static constexpr int  height		= screen_height + 2 * v_border; // height of bits[]

		if unlikely (videoframe->max_width != width || videoframe->max_height != height) //
			videoframe->resize(width, height);
	}

	assert(videoframe->max_width >= 256);
	assert(videoframe->max_height >= 192);
	assert(videoframe->max_width % 8 == 0);

	assert(newdata->screen.left() % 8 == 0);
	assert(newdata->screen.width() % 8 == 0);
	assert(newdata->frame.width % 8 == 0);

	assert_ge(newdata->screen.left(), 0);
	assert_gt(newdata->screen.width(), 0);
	assert_le(newdata->screen.left() + newdata->screen.width(), newdata->frame.width);
	assert_ge(newdata->screen.top(), 0);
	assert_gt(newdata->screen.height(), 0);
	assert_le(newdata->screen.top() + newdata->screen.height(), newdata->frame.height);

	// provide colormap for GifRecorder:
	videoframe->cmap	 = &zxsp_colormap;
	videoframe->flashing = false;

	// crop frame to videoframe.fb_size:
	int fb_height	  = videoframe->max_height;
	int top_border	  = newdata->screen.top();
	int screen_height = min(newdata->screen.height(), fb_height);
	int bottom_border = newdata->frame.height - top_border - screen_height;

	bottom_border = min(bottom_border, (fb_height - screen_height) / 2);
	top_border	  = min(top_border, (fb_height - screen_height + 1) / 2);
	fb_height	  = top_border + screen_height + bottom_border;

	int fb_width	 = videoframe->max_width;
	int left_border	 = newdata->screen.left();
	int screen_width = min(newdata->screen.width(), fb_width);
	int right_border = newdata->frame.width - left_border - screen_width;

	right_border = min(right_border, (fb_width - screen_width) / 2);
	left_border	 = min(left_border, (fb_width - screen_width) / 2);
	fb_width	 = left_border + screen_width + right_border;

	// store frame size and screen rect:
	videoframe->frame  = {fb_width, fb_height};
	videoframe->screen = {left_border, top_border, screen_width, screen_height}; // {xywh}


	// copy frame buffer:

	// source and destination pointers:
	int			 bpr = newdata->frame.width / 8; // bytes per row (source)
	Color*		 zp	 = videoframe->pixels;
	const uint8* qp	 = newdata->pixel_octets +							 //
					  2 * ((newdata->screen.top() - top_border) * bpr) + //
					  2 * ((newdata->screen.left() - left_border) / 8);	 // round down if (left_border&4) != 0

	int l = left_border & 4;  // left border nibble is read and skipped => account in w
	int r = right_border & 4; // right border nibble is read and not skipped => not account in w
	int w = (left_border + l + screen_width + right_border) / 8; // source bytes read per row
	int d = bpr - w;											 // offset to skip gap to next row

	//copy_framebuffer(videoframe->frame, qp, zp, d, l, r);

	for (int y = 0; y < videoframe->frame.height; y++)
	{
		Color* ze = zp + (videoframe->frame.width - r);

		if (l)
		{
			uint pixels		 = *qp++ & 15; // read and skip nibble  (lower nibble in pixels)
			uint attr		 = *qp++;	   // read and skip nibble
			uint pen_color	 = color<Color>(attr & 15);
			uint paper_color = color<Color>(attr >> 4);
			for (int m = 0x08; m; m = m >> 1) { *zp++ = pixels & m ? pen_color : paper_color; }
		}

		while (zp < ze)
		{
			uint8 pixels	  = *qp++;
			uint8 attr		  = *qp++;
			uint  pen_color	  = color<Color>(attr & 15);
			uint  paper_color = color<Color>(attr >> 4);

			if (pixels == 0xff)
			{
				for (int i = 0; i < 8; i++) *zp++ = pen_color;
			}
			else if (pixels == 0x00)
			{
				for (int i = 0; i < 8; i++) *zp++ = paper_color;
			}
			else
			{
				for (int m = 0x80; m; m = m >> 1) { *zp++ = pixels & m ? pen_color : paper_color; }
			}
		}

		if (r) // read and not skip nibble:
		{
			uint pixels		 = qp[0] >> 4; // read and not skip nibble  (higher nibble in pixels)
			uint attr		 = qp[1];	   // read and not skip nibble
			uint pen_color	 = color<Color>(attr & 15);
			uint paper_color = color<Color>(attr >> 4);
			for (int m = 0x08; m; m = m >> 1) { *zp++ = pixels & m ? pen_color : paper_color; }
		}
		qp += 2 * d; // skip to next row start
	}

	//assert(zp == videoframe->pixels + fb_width * fb_height);


	// videobeam indicator:
	//	static constexpr int pixel_per_cc = 2;
	//	int	  cc_screen	  = q_screen_width / pixel_per_cc; // 128 -> 256 pixel
	//	int	  cc_h_border = q_left_border / pixel_per_cc;  // 32  -> 64 pixel
	//	int	  cc_per_scanline		 = newdata->cc_per_scanline;
	//	int32 cc_start_of_screenfile = newdata->cc_start_of_screenfile;
	//	int32 cc_vbi = newdata->cc;
	//	if (cc_vbi) TODO();
}

#if 0
  #define B black
  #define W white
static constexpr RgbaColor four_rgba_pixels[16][4] = {
	{B, B, B, B}, {B, B, B, W}, {B, B, W, B}, {B, B, W, W}, //
	{B, W, B, B}, {B, W, B, W}, {B, W, W, B}, {B, W, W, W}, //
	{W, B, B, B}, {W, B, B, W}, {W, B, W, B}, {W, B, W, W}, //
	{W, W, B, B}, {W, W, B, W}, {W, W, W, B}, {W, W, W, W}, //
};
  #undef B
  #undef W

static constexpr uint8 four_ic_pixels[16][4] = {
	{0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 1, 1}, //
	{0, 1, 0, 0}, {0, 1, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 1}, //
	{1, 0, 0, 0}, {1, 0, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 1}, //
	{1, 1, 0, 0}, {1, 1, 0, 1}, {1, 1, 1, 0}, {1, 1, 1, 1}, //
};

template<typename Color>
inline constexpr Color color(int index)
{
	return index ? white : black;
}
template<>
inline constexpr uint8 color<uint8>(int index)
{
	return index;
}
template<typename Color>
inline constexpr const Color* four_pixels(int nibble)
{
	return four_rgba_pixels[nibble];
}
template<>
inline constexpr const uint8* four_pixels<uint8>(int nibble)
{
	return four_ic_pixels[nibble];
}


using GifColor							= uint8;
static constexpr Comp	  B				= (black >> 8) & 0xff;
static constexpr Comp	  W				= (white >> 8) & 0xff;
static constexpr GifColor transp		= 2; // transparent color index
static constexpr Comp	  zx80_colors[] = {B, B, B, W, W, W, 0, 0, 0};
static const Colormap	  zx80_colormap(zx80_colors, 3, transp);

//static_assert(native_byteorder == little_endian, "");
//#define C(a, b, c, d) uint32(a + (b << 8) + (c << 16) + (d << 24))
//static constexpr uint32 four_ic_pixels[16] = {
//	C(0, 0, 0, 0), C(0, 0, 0, 1), C(0, 0, 1, 0), C(0, 0, 1, 1), //
//	C(0, 1, 0, 0), C(0, 1, 0, 1), C(0, 1, 1, 0), C(0, 1, 1, 1), //
//	C(1, 0, 0, 0), C(1, 0, 0, 1), C(1, 0, 1, 0), C(1, 0, 1, 1), //
//	C(1, 1, 0, 0), C(1, 1, 0, 1), C(1, 1, 1, 0), C(1, 1, 1, 1), //
//};
//#undef C


template<typename Color>
void zx80Renderer(VideoFrame<Color>* videoframe, VideoData* newframedata)
{
	// render b&w video image in newframedata into the output videoframe.
	// the input frame is cropped to fit in the limits of the videoframe.
	// the final frame size and screen rect are stored into the videoframe.
	// the pixels from the input frame are rendered into the videoframe.

	assert(newframedata->what == VideoData::Zx80Frame);
	Zx80VideoData* newdata = reinterpret_cast<Zx80VideoData*>(newframedata);

	assert(videoframe->max_width >= 256);
	assert(videoframe->max_height >= 192);
	assert(videoframe->max_width % 8 == 0);

	assert(newdata->screen.left() % 8 == 0);
	assert(newdata->screen.width() % 8 == 0);
	assert(newdata->frame.width % 8 == 0);

	assert(newdata->screen.left() >= 0);
	assert(newdata->screen.width() > 0);
	assert(newdata->screen.left() + newdata->screen.width() <= newdata->frame.width);
	assert(newdata->screen.top() >= 0);
	assert(newdata->screen.height() > 0);
	assert(newdata->screen.top() + newdata->screen.height() <= newdata->frame.height);

	// provide colormap for GifRecorder:
	videoframe->cmap	 = &zx80_colormap;
	videoframe->flashing = false;

	// crop frame to videoframe.fb_size:
	int fb_height	  = videoframe->max_height;
	int top_border	  = newdata->screen.top();
	int screen_height = min(newdata->screen.height(), fb_height);
	int bottom_border = newdata->frame.height - top_border - screen_height;

	bottom_border = min(bottom_border, (fb_height - screen_height) / 2);
	top_border	  = min(top_border, (fb_height - screen_height + 1) / 2);
	fb_height	  = top_border + screen_height + bottom_border;

	int fb_width	 = videoframe->max_width;
	int left_border	 = newdata->screen.left();
	int screen_width = min(newdata->screen.width(), fb_width);
	int right_border = newdata->frame.width - left_border - screen_width;

	right_border = min(right_border, (fb_width - screen_width) / 2);
	left_border	 = min(left_border, (fb_width - screen_width) / 2);
	fb_width	 = left_border + screen_width + right_border;

	// store frame size and screen rect:
	videoframe->frame  = {fb_width, fb_height};
	videoframe->screen = {left_border, top_border, screen_width, screen_height}; // {xywh}


	// copy frame buffer:

	enum : Color {
		black = color<Color>(0), //
		white = color<Color>(1)	 //
	};

	// source and destination pointers:
	int			 bpr = newdata->frame.width / 8; // bytes per row (source)
	Color*		 zp	 = videoframe->pixels;
	const uint8* qp	 = newdata->pixel_octets +					   //
					  (newdata->screen.top() - top_border) * bpr + //
					  (newdata->screen.left() - left_border) / 8;  // round down if (left_border&4) != 0

	int l = left_border & 4;  // left border nibble is read and skipped => account in w
	int r = right_border & 4; // right border nibble is read and not skipped => not account in w
	int w = (left_border + l + screen_width + right_border) / 8; // source bytes read per row
	int d = bpr - w;											 // offset to skip gap to next row

	//copy_framebuffer(videoframe->frame, qp, zp, d, l, r);

	for (int y = 0; y < videoframe->frame.height; y++)
	{
		Color* ze = zp + (videoframe->frame.width - r);

		if (l)
		{
			const Color* q = four_pixels<Color>(*qp++ & 15); // read and skip nibble
			for (int i = 0; i < 4; i++) *zp++ = *q++;
		}

		for (uint8 octet = *qp++;;)
		{
			if (octet == 0xff) do // speed-up
				{
					for (int i = 0; i < 8; i++) *zp++ = white;
					if (zp >= ze) goto row_end;
				}
				while ((octet = *qp++) == 0xff);

			if (octet == 0x00) do // speed_up
				{
					for (int i = 0; i < 8; i++) *zp++ = black;
					if (zp >= ze) goto row_end;
				}
				while ((octet = *qp++) == 0x00);

			do {
				const Color* q = four_pixels<Color>(octet >> 4);
				for (int i = 0; i < 4; i++) *zp++ = *q++;
				q = four_pixels<Color>(octet & 15);
				for (int i = 0; i < 4; i++) *zp++ = *q++;
				if (zp >= ze) goto row_end;
			}
			while ((octet = *qp++) != 0x00 && octet != 0xff);
		}

	row_end:
		if (r)
		{
			const Color* q = four_pixels<Color>((*qp) >> 4); // read and not skip nibble
			for (int i = 0; i < 4; i++) *zp++ = *q++;
		}
		qp += d; // skip to next row start
	}

	//assert(zp == videoframe->pixels + fb_width * fb_height);


	// videobeam indicator:
	//	static constexpr int pixel_per_cc = 2;
	//	int	  cc_screen	  = q_screen_width / pixel_per_cc; // 128 -> 256 pixel
	//	int	  cc_h_border = q_left_border / pixel_per_cc;  // 32  -> 64 pixel
	//	int	  cc_per_scanline		 = newdata->cc_per_scanline;
	//	int32 cc_start_of_screenfile = newdata->cc_start_of_screenfile;
	//	int32 cc_vbi = newdata->cc;
	//	if (cc_vbi) TODO();
}
#endif

template void zx80Renderer(VideoFrame<RgbaColor>* videoframe, VideoData* newframedata);
template void zx80Renderer(VideoFrame<uint8>* videoframe, VideoData* newframedata);


} // namespace zxsp


/*


























*/
