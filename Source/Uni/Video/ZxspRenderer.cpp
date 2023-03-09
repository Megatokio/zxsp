// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxspRenderer.h"
#include "OS/DspTime.h"
#include "Templates/Array.h"
#include "graphics/gif/GifEncoder.h"
#include "unix/os_utilities.h"
#include "version.h"


#define opacity 0xFFFFFFFF // e.g. 0xFFFFFF80 for fading out tv image

const RgbaColor zxsp_rgba_colors[16] = // RGBA
	{opacity & black,		opacity& blue,		  opacity& red,			  opacity& magenta,
	 opacity& green,		opacity& cyan,		  opacity& yellow,		  opacity& white,
	 opacity& bright_black, opacity& bright_blue, opacity& bright_red,	  opacity& bright_magenta,
	 opacity& bright_green, opacity& bright_cyan, opacity& bright_yellow, opacity& bright_white};


/*	rendere Ausgaben der Zxsp Ula in bits[].

	ioinfo[]: alle OUTs zur ULA
		-> set Border
		ioinfo[] muss mit einem OUT anfangen, das die Bordercolor setzt!
		ioinfo[] muss am Ende noch einen freien Platz haben!

	attr_pixels[192*32]: von der ULA ausgegebene Attribut/Pixel-Pärchen
		pro Scanline werden 32 Pärchen (64 Bytes) ausgegeben
*/
void ZxspRenderer::drawScreen(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase, uint32 cc_vbi)
{
	assert((cc_start_of_screenfile & 3) == 0);

	cc_start_of_screenfile += 4;
	cc_vbi = (cc_vbi + 3) & ~3;

	int32 cc_row_flyback			 = cc_per_scanline - cc_screen - 2 * cc_h_border;
	int32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	if (int32(cc_vbi) < cc_start_of_visible_screen)
		return; // video beam ist noch über dem sichtbaren Bildschirmausschnitt
	int32 cc_end_of_visible_screen =
		min(int32(cc_vbi), cc_start_of_visible_screen + int(height) * int(cc_per_scanline) - cc_row_flyback);

	// draw border and screenfile:

	RgbaColor bordercolor = black;

	int32 cc_io			   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	RgbaColor* p		   = bits;										// current pixel pointer
	RgbaColor* a		   = bits;										// current start of row
	uint	   row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < int32(cc_end_of_visible_screen); io++)
	{
		if (io->addr & 1) continue; // no ula address

		if (int32(io->cc) > cc_start_of_visible_screen)
		{
			int32 cc = min(cc_end_of_visible_screen, int32(io->cc + 3) & ~3) - cc_start_of_visible_screen;

			int end_row = cc / cc_per_scanline;
			int end_col = min(uint(width), cc % cc_per_scanline * pixel_per_cc);
			assert(end_row < height);
			RgbaColor* ee = bits + end_row * width + end_col; // cc_io end pointer
			RgbaColor* e;									  // intermediate ent pointers

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

						uint pen_color	 = zxsp_rgba_colors[(attr & 7) + ((attr >> 3) & 8)];
						uint paper_color = zxsp_rgba_colors[(attr >> 3) & 15];

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

		cc_io		= io->cc;
		bordercolor = zxsp_rgba_colors[io->byte & 7];
	}

	if (p < bits + width * height) // Video beam indicator
	{
		assert(p <= bits + width * height - 8);
		RgbaColor c = int(system_time * 6) & 1 ? bright_yellow : bright_red;
		for (int i = 0; i < 8; i++) p[i] = c;
		return;
	}

	assert(p == bits + width * height);
}


// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================


using GifColor = uint8;

/* global ZX Spectrum color table:
		8 x normal brightness
		8 x bright
*/
const GifColor transp	   = 8;	   // 'bright black' used for transparency
const Comp	   F		   = 0xFF; // "bright": full brightness
const Comp	   H		   = 0xCC; // "normal": reduced brightness: 80%
const Comp	   zx_colors[] = {0, 0, 0, 0, 0, H, H, 0, 0, H, 0, H, 0, H, 0, 0, H, H, H, H, 0, H, H, H, // r,g,b
							  0, 0, 0, 0, 0, F, F, 0, 0, F, 0, F, 0, F, 0, 0, F, F, F, F, 0, F, F, F};
const Colormap zxsp_colormap(zx_colors, 16, transp);


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
