// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"
#include "Templates/Array.h"
#include "graphics/gif/GifEncoder.h"
#include "unix/os_utilities.h"
#include "version.h"
#include "zxsp_globals.h"


namespace zxsp
{


constexpr RgbaColor zxsp_rgba_colors[16] = { // RGBA
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
	return zxsp_rgba_colors[index];
}
template<>
inline constexpr uint8 color<uint8>(int index)
{
	return index;
}


/*	rendere Ausgaben der Zxsp Ula in bits[].

	ioinfo[]: alle OUTs zur ULA
		-> set Border
		ioinfo[] muss mit einem OUT anfangen, das die Bordercolor setzt!
		ioinfo[] muss am Ende noch einen freien Platz haben!

	attr_pixels[192*32]: von der ULA ausgegebene Attribut/Pixel-Pärchen
		pro Scanline werden 32 Pärchen (64 Bytes) ausgegeben
*/
template<typename Color>
void zxspRenderer(VideoFrame<Color>* videoframe, VideoData* _data)
{
	// the zxspRenderer is used to divert to the actually needed renderer:
	switch (_data->what)
	{
	default: TODO();
	case VideoData::Zx80Frame: return zx80Renderer(videoframe, _data);
	case VideoData::Tc2048Frame: return tc2048Renderer(videoframe, _data);
	case VideoData::SpectraFrame: return spectraRenderer(videoframe, _data);
	case VideoData::ZxspFrame: break;
	}

	static constexpr bool is_ic = sizeof(Color) == 1;

	ZxspVideoData* newdata = reinterpret_cast<ZxspVideoData*>(_data);
	videoframe->cmap	   = &zxsp_colormap; // for GifRecorder

	static constexpr int screen_width  = 256;
	static constexpr int screen_height = 192;
	static constexpr int h_border	   = is_ic ? 32 : 6 * 8;		   // pixels, must be N*8
	static constexpr int v_border	   = is_ic ? 24 : 6 * 6;		   // pixels
	static constexpr int width		   = screen_width + 2 * h_border;  // width of bits[]
	static constexpr int height		   = screen_height + 2 * v_border; // height of bits[]

	static constexpr int pixel_per_cc = 2;
	static constexpr int cc_screen	  = screen_width / pixel_per_cc; // 128 -> 256 pixel
	static constexpr int cc_h_border  = h_border / pixel_per_cc;	 // 32  -> 64 pixel

	if unlikely (videoframe->max_width != width || videoframe->max_height != height) //
		videoframe->resize(width, height);

	//videoframe->frame  = Size {width, height};
	//videoframe->screen = Rect {h_border, v_border, screen_width, screen_height};

	assert_eq(videoframe->frame.width, width);
	assert_eq(videoframe->frame.height, height);
	assert(videoframe->screen.top() == v_border);
	assert(videoframe->screen.left() == h_border);
	assert(videoframe->screen.width() == screen_width);
	assert(videoframe->screen.height() == screen_height);

	int	  cc_per_scanline		 = newdata->cc_per_scanline;
	int32 cc_start_of_screenfile = newdata->cc_start_of_screenfile;
	int32 cc_vbi				 = newdata->cc;

	assert((cc_start_of_screenfile & 3) == 0);

	cc_start_of_screenfile += 4;
	cc_vbi = (cc_vbi + 3) & ~3;

	int32 cc_row_flyback			 = cc_per_scanline - cc_screen - 2 * cc_h_border;
	int32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	if (cc_vbi < cc_start_of_visible_screen) return; // video beam is still above visible border
	int32 cc_end_of_visible_screen =
		min(cc_vbi, cc_start_of_visible_screen + height * cc_per_scanline - cc_row_flyback);

	// draw border and screenfile:

	enum : Color {
		black		   = color<Color>(0),
		blue		   = color<Color>(1),
		red			   = color<Color>(2),
		magenta		   = color<Color>(3),
		green		   = color<Color>(4),
		cyan		   = color<Color>(5),
		yellow		   = color<Color>(6),
		white		   = color<Color>(7),
		bright_black   = color<Color>(8),
		bright_blue	   = color<Color>(9),
		bright_red	   = color<Color>(10),
		bright_magenta = color<Color>(11),
		bright_green   = color<Color>(12),
		bright_cyan	   = color<Color>(13),
		bright_yellow  = color<Color>(14),
		bright_white   = color<Color>(15)
	};

	Color	bordercolor	 = black;
	bool	flashphase	 = newdata->flashphase;
	IoInfo* ioinfo		 = newdata->ioinfo;
	int		ioinfo_count = newdata->ioinfo_count;
	assert(ioinfo_count < newdata->ioinfo_size);
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	int32  cc_io		   = cc_start_of_visible_screen;
	Color* p			   = videoframe->pixels;  // current pixel pointer
	Color* a			   = videoframe->pixels;  // current start of row
	uint   row			   = 0;					  // current row in bits[]
	uint8* q			   = newdata->attrpixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if (io->addr & 1) continue; // no ula address

		if (io->cc > cc_start_of_visible_screen)
		{
			int32 cc = min(cc_end_of_visible_screen, (io->cc + 3) & ~3) - cc_start_of_visible_screen;

			int end_row = cc / cc_per_scanline;
			int end_col = min(width, cc % cc_per_scanline * pixel_per_cc);
			assert(end_row < height);
			Color* ee = videoframe->pixels + end_row * width + end_col; // cc_io end pointer
			Color* e;													// intermediate end pointers

			// draw all border pixels up to cc_io:
			while (p < ee)
			{
				if (row >= v_border && row < v_border + screen_height) // if inside screenfile region:
				{
					e = min(a + h_border, ee);
					while (p < e) { *p++ = bordercolor; } // draw left border
					if (p < a + h_border) break;		  // exit pixel loop if at cc_io

					// draw screen row:
					e = min(ee, e + screen_width);
					while (p < e)
					{
						uint pixels = *q++;
						uint attr	= *q++;

						if (attr & 0x80 && flashphase) pixels ^= 0xff;

						Color pen_color	  = color<Color>((attr & 7) + ((attr >> 3) & 8));
						Color paper_color = color<Color>((attr >> 3) & 15);

						for (int m = 0x80; m; m = m >> 1) { *p++ = pixels & m ? pen_color : paper_color; }
					}
				}

				// draw (remainder of) screen row
				e = min(a + width, ee);
				while (p < e) { *p++ = bordercolor; }

				if (p == a + width)
				{
					a = p;
					row++;
				} // advance row number if end of row reached
			}
		}

		cc_io		= max(cc_io, io->cc);
		bordercolor = color<Color>(io->byte & 7);
	}

	assert(p <= videoframe->pixels + width * height);

	if (p < videoframe->pixels + width * height) // Video beam indicator
	{
		assert(p <= videoframe->pixels + width * height - 8);
		Color c = int(system_time * 6) & 1 ? bright_yellow : bright_red;
		for (int i = 0; i < 8; i++) p[i] = c;
	}
}

// instantiate:
template void zxspRenderer(VideoFrame<RgbaColor>* videoframe, VideoData* newframedata);
template void zxspRenderer(VideoFrame<uint8>* videoframe, VideoData* newframedata);


// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================


//using GifColor = uint8;

///* global ZX Spectrum color table:
//		8 x normal brightness
//		8 x bright
//*/
//const GifColor transp	   = 8;	   // 'bright black' used for transparency
//const Comp	   F		   = 0xFF; // "bright": full brightness
//const Comp	   H		   = 0xCC; // "normal": reduced brightness: 80%
//const Comp	   zx_colors[] = {0, 0, 0, 0, 0, H, H, 0, 0, H, 0, H, 0, H, 0, 0, H, H, H, H, 0, H, H, H, // r,g,b
//							  0, 0, 0, 0, 0, F, F, 0, 0, F, 0, F, 0, F, 0, 0, F, F, F, F, 0, F, F, F};
//const Colormap zxsp_colormap(zx_colors, 16, transp);

#if 0

ZxspGifWriter::ZxspGifWriter(bool update_border, uint frames_per_second) :
	GifWriter(isa_ZxspGifWriter, zxsp_colormap, 256, 192, 32, 24, update_border, frames_per_second)
{}

ZxspGifWriter::ZxspGifWriter(isa_id id, const Colormap& colormap, bool update_border, uint frames_per_second) :
	GifWriter(id, colormap, 256, 192, 32, 24, update_border, frames_per_second)
{}


void ZxspGifWriter::drawScreen(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase)
{
	assert((cc_start_of_screenfile & 3) == 0);
	if (!bits) bits = new Pixelmap(width, height);

	cc_start_of_screenfile += 4;

	uint32 cc_row_flyback			  = cc_per_scanline - cc_screen - 2 * cc_h_border;
	uint32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	uint32 cc_end_of_visible_screen	  = cc_start_of_visible_screen + height * cc_per_scanline - cc_row_flyback;

	// draw border:
	// and screenfile:

	GifColor bordercolor = 0; /*black*/

	uint32 cc_io		   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	GifColor* p			   = bits->getData();							// current pixel pointer
	GifColor* a			   = bits->getData();							// current start of row
	uint	  row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if (io->addr & 1) continue; // no ula address

		if (io->cc > cc_start_of_visible_screen)
		{
			uint32 cc = min(cc_end_of_visible_screen, (io->cc + 3) & ~3) - cc_start_of_visible_screen;

			uint end_row = cc / cc_per_scanline;
			uint end_col = min(uint(width), cc % cc_per_scanline * pixel_per_cc);
			assert(end_row < height);
			GifColor* ee = bits->getData() + end_row * width + end_col; // cc_io end pointer
			GifColor* e;												// intermediate ent pointers

			// draw all border pixels up to cc_io:
			while (p < ee)
			{
				if (row >= v_border && row < v_border + screen_height) // if inside screenfile region:
				{
					e = min(a + h_border, ee);
					while (p < e) { *p++ = bordercolor; } // draw left border
					if (p < a + h_border) break;		  // exit pixel loop if at cc_io

					// draw screen row:
					for (e += screen_width; p < e;)
					{
						uint pixels = *q++;
						uint attr	= *q++;

						if (attr & 0x80 && flashphase) pixels ^= 0xff;

						uint pen_color = (attr & 7) + ((attr >> 3) & 8);
						if (pen_color == transp) pen_color = 0;
						uint paper_color = (attr >> 3) & 15;
						if (paper_color == transp) paper_color = 0;

						for (uint m = 0x80; m; m = m >> 1) { *p++ = pixels & m ? pen_color : paper_color; }
					}
				}

				// draw (remainder of) screen row
				e = min(a + width, ee);
				while (p < e) { *p++ = bordercolor; }

				if (p == a + width)
				{
					a = p;
					row++;
				} // advance row number if end of row reached
			}
		}

		cc_io		= io->cc;
		bordercolor = io->byte & 7;
	}

	assert(p == bits->getData() + width * height);
}


/*	append frame to a gif movie
	this is the version for ZX Spectrum-style screens
*/
void ZxspGifWriter::writeFrame(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase)
{
	assert(gif_encoder.imageInProgress());
	assert(bits && bits2 && diff && diff2);

	if (update_border || frame_count == 0) bits->setFrame(0, 0, width, height);
	else bits->setFrame(h_border, v_border, screen_width, screen_height);

	drawScreen(
		ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, flashphase); // bits := new screen
	*diff = *bits;

	if (frame_count == 0) {} // first screen
	else					 // subsequent screen
	{
		diff->reduceToDiff(*bits2, global_colormap.transpColor());
		if (diff->isEmpty()) // no change -> increase duration
		{
			frame_count++;
			return;
		}
		else // screen changed -> write old screen to file
		{
			write_diff2_to_file();
		}
	}

	std::swap(bits, bits2);
	std::swap(diff, diff2);
	frame_count = 1;
}


/*	Save the screenshot already rendered in bits[] and bits2[] to file
	this is the version for ZX Spectrum-style screens
*/
void ZxspGifWriter::saveScreenshot(
	cstr path, IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline,
	uint32 cc_start_of_screenfile)
{
	assert(!gif_encoder.imageInProgress());
	assert(!bits && !bits2); // else we'd need to fix the bbox

	drawScreen(ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, 0);
	std::swap(bits, bits2);
	drawScreen(ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, 1);
	bits2->reduceToDiff(*bits, global_colormap.transpColor());

	// Calculate Colormaps and total number of colors in image:
	Colormap cmap = global_colormap;
	bits->reduceColors(cmap);
	Colormap cmap2 = global_colormap;
	bits2->reduceColors(cmap2);
	uint total_colors = cmap2.usedColors();
	for (int i = 0; i < cmap.usedColors(); i++) { total_colors += cmap2.findColor(cmap[i]) == Colormap::not_found; }

	// Write to file:
	gif_encoder.openFile(path);
	gif_encoder.writeScreenDescriptor(width, height, total_colors, 0 /*aspect_ratio*/);
	gif_encoder.writeCommentBlock(
		usingstr("created on %s by %s with %s %s\n", datestr(now()), getUser(), APPL_NAME, APPL_VERSION_STR));

	if (bits2->isEmpty()) // no flashing bits
	{
		gif_encoder.writeImage(*bits, cmap);
	}
	else // flashing
	{
		uint delay = (frames_per_flashphase * 100 + frames_per_second / 2) / frames_per_second;
		gif_encoder.writeLoopingAnimationExtension();
		gif_encoder.writeGraphicControlBlock(delay);
		gif_encoder.writeImage(*bits, cmap);
		gif_encoder.writeGraphicControlBlock(delay, cmap2.transpColor());
		gif_encoder.writeImage(*bits2, cmap2);
	}

	gif_encoder.closeFile();

	delete bits;
	bits = nullptr;
	delete bits2;
	bits2 = nullptr;
}

#endif


} // namespace zxsp
