// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"

namespace zxsp
{

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


#if 0
static inline void copy_framebuffer(Size fb, const uint8* qp, uint8* _zp, int d, int l, int r)
{
	assert((size_t(_zp) & 3) == 0);
	assert((r | 4) == 4);
	assert((l | 4) == 4);

	uint32* zp = reinterpret_cast<uint32*>(_zp);

	const uint32 white = 0xffffffff;
	const uint32 black = 0x000000ff;

	for (int y = 0; y < fb.height; y++)
	{
		uint32* ze = zp + (fb.width - r) / 4;

		if (l)
		{
			*zp++ = four_ic_pixels[*qp++ & 15]; // read and skip nibble
		}

		for (uint8 octet = *qp++;;)
		{
			if (octet == 0xff) do // speed-up
				{
					*zp++ = white;
					*zp++ = white;
					if (zp >= ze) goto row_end;
				}
				while ((octet = *qp++) == 0xff);

			if (octet == 0x00) do // speed_up
				{
					*zp++ = black;
					*zp++ = black;
					if (zp >= ze) goto row_end;
				}
				while ((octet = *qp++) == 0x00);

			do {
				*zp++ = four_ic_pixels[octet >> 4];
				*zp++ = four_ic_pixels[octet & 15];
				if (zp >= ze) goto row_end;
			}
			while ((octet = *qp++) != 0x00 && octet != 0xff);
		}

	row_end:
		if (r)
		{
			*zp++ = four_ic_pixels[(*qp) >> 4]; // read and not skip nibble
		}
		qp += d; // skip to next row start
	}

	assert(u8ptr(zp) == _zp + fb.width * fb.height);
}

static inline void copy_framebuffer(Size fb, const uint8* qp, RgbaColor* zp, int d, int l, int r)
{
	for (int y = 0; y < fb.height; y++)
	{
		RgbaColor* ze = zp + (fb.width - r);

		if (l)
		{
			const RgbaColor* q = four_rgba_pixels[*qp++ & 15]; // read and skip nibble
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
				const RgbaColor* q = four_rgba_pixels[octet >> 4];
				for (int i = 0; i < 4; i++) *zp++ = *q++;
				q = four_rgba_pixels[octet & 15];
				for (int i = 0; i < 4; i++) *zp++ = *q++;
				if (zp >= ze) goto row_end;
			}
			while ((octet = *qp++) != 0x00 && octet != 0xff);
		}

	row_end:
		if (r)
		{
			const RgbaColor* q = four_rgba_pixels[(*qp) >> 4]; // read and not skip nibble
			for (int i = 0; i < 4; i++) *zp++ = *q++;
		}
		qp += d; // skip to next row start
	}

	//assert(zp == videoframe->pixels + fb_width * fb_height);
}
#endif


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

template void zx80Renderer(VideoFrame<RgbaColor>* videoframe, VideoData* newframedata);
template void zx80Renderer(VideoFrame<uint8>* videoframe, VideoData* newframedata);


} // namespace zxsp


/*


























*/
