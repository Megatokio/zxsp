// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Tc2048Renderer.h"
#include "Ula/UlaTc2048.h"


/*	TS2068:
	Port 0xFF: set display mode:
	byte = %BIaaavvv

		Bits vvv: Video Mode:
		000 = Primary DFILE active		standard zxsp screen at 0x4000-0x5aff
		001 = Secondary DFILE active	standard zxsp screen at 0x6000-0x7aff
		010 = Extended Colour Mode		chars at 0x4000-0x57ff, colors at 0x6000-0x7aff
		110 = 64 column mode			columns 0,2,4,...62 from DFILE 1
										columns 1,3,5,...63 from DFILE 2
		other = unpredictable results

		Bits aaa: ink/paper selection for 64 column mode	(zxsp screen attributes in brackets)
		000 = Black/White		(56)
		001 = Blue/Yellow		(49)
		010 = Red/Cyan			(42)
		011 = Magenta/Green		(35)
		100 = Green/Magenta		(28)
		101 = Cyan/Red			(21)
		110 = Yellow/Blue		(14)
		111 = White/Black		(7)
*/


/*	rendere Ausgaben der Tc2048 Ula in bits[].

	ioinfo[]: alle OUTs zur ULA
		-> set Border
		-> set Screen Mode
		ioinfo[] muss mit zwei OUTs anfangen, die Screenmode Bordercolor setzen!
		ioinfo[] muss am Ende noch einen freien Platz haben!

	attr_pixels[192*32]: von der ULA ausgegebene Attribut/Pixel-Pärchen
		pro Scanline werden 32 Pärchen (64 Bytes) ausgegeben
		32 column mode: high byte = pixels;      low byte = attr
		64 column mode: high byte = left pixels; low byte = right pixels
*/
void Tc2048Renderer::drawScreen(
	IoInfo* ioinfo,
	uint	ioinfo_count,
	uint8*	attr_pixels,
	uint	cc_per_scanline,
	uint32	cc_start_of_screenfile,
	bool	flashphase,
	uint32	cc_vbi)
{
	assert((cc_start_of_screenfile & 3) == 0);

	cc_start_of_screenfile += 4;
	cc_vbi = (cc_vbi + 3) & ~3;

	int32 cc_row_flyback			 = cc_per_scanline - screen_width / pixel_per_cc - 2 * cc_h_border;
	int32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	if (int32(cc_vbi) < cc_start_of_visible_screen)
		return; // video beam ist noch über dem sichtbaren Bildschirmausschnitt
	int32 cc_end_of_visible_screen =
		min(int(cc_vbi), cc_start_of_visible_screen + int(height) * int(cc_per_scanline) - cc_row_flyback);

	// draw border and screenfile:

	RgbaColor mode32_bordercolor = black;
	RgbaColor bordercolor		 = black;
	bool	  mode64			 = no;

	RgbaColor paper_color = bordercolor;			  // mode64
	RgbaColor pen_color	  = paper_color ^ 0xffffff00; // mode64

	int32 cc_io			   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	RgbaColor* p		   = bits; // current pixel pointer	// GIF RENDERER: anpassen!
	RgbaColor* a		   = bits; // current start of row		// GIF RENDERER: anpassen!
	uint	   row		   = 0;	   // current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if ((io->addr & 0xfe) != 0xfe) continue; // no ula address

		if (int(io->cc) > cc_start_of_visible_screen)
		{
			int32 cc = min(cc_end_of_visible_screen, int(io->cc + 3) & ~3) - cc_start_of_visible_screen;

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
					if (mode64) // 64 column mode
					{
						e = min(ee, e + screen_width);
						while (p < e)
						{
							uint pixels = *q++;
							for (uint m = 0x80; m; m = m >> 1) { *p++ = pixels & m ? pen_color : paper_color; }
						}
					}
					else // 32 column mode
					{
						e = min(ee, e + screen_width);
						while (p < e)
						{
							uint pixels = *q++;
							uint attr	= *q++;

							if (attr & 0x80 && flashphase) pixels ^= 0xff;

							pen_color	= zxsp_rgba_colors[(attr & 7) + ((attr >> 3) & 8)];
							paper_color = zxsp_rgba_colors[(attr >> 3) & 15];

							for (uint m = 0x80; m; m = m >> 1)
							{
								if (pixels & m)
								{
									*p++ = pen_color;
									*p++ = pen_color;
								}
								else
								{
									*p++ = paper_color;
									*p++ = paper_color;
								}
							}
						}
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

		cc_io = io->cc;

		if (io->addr & 1) // out(0xff)
		{
			mode64		= io->byte & 4;
			pen_color	= zxsp_rgba_colors[8 + ((io->byte >> 3) & 7)];
			paper_color = pen_color ^ 0xffffff00;
			bordercolor = mode64 ? paper_color : mode32_bordercolor;
		}
		else // out(0xfe)
		{
			mode32_bordercolor = zxsp_rgba_colors[io->byte & 7];
			if (!mode64) bordercolor = mode32_bordercolor;
		}
	}

	if (p < bits + width * height) // Video beam indicator
	{
		assert(p <= bits + width * height - 16);
		RgbaColor c = int(system_time * 6) & 1 ? bright_yellow : bright_red;
		for (uint i = 0; i < 16; i++) p[i] = c;
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
		1 x 50% grau für TC2048 64 Column-Mode
*/
const GifColor transp	   = 8;	   // 'bright black' used for transparency
const Comp	   F		   = 0xFF; // "bright": full brightness
const Comp	   H		   = 0xCC; // "normal": reduced brightness: 80%
const Comp	   tc_colors[] = {0, 0, 0, 0, 0, H, H, 0, 0, H, 0, H, 0, H, 0, 0, H, H, H, H, 0, H, H, H, // r,g,b
							  0, 0, 0, 0, 0, F, F, 0, 0, F, 0, F, 0, F, 0, 0, F, F, F, F, 0, F, F, F, 128, 128, 128};
const Colormap tc2048_colormap(tc_colors, 17, transp);


Tc2048GifWriter::Tc2048GifWriter(QObject* p, bool update_border) :
	ZxspGifWriter(p, isa_Tc2048GifWriter, tc2048_colormap, update_border, 50)
{}


/*

	*** Da niemand mehr das Pixel-Seitenverhältnis beachtet, kann kein Gif mit 512x192 Pixeln erzeugt werden.
		Man müsste also alle Zeilen verdoppeln.
		Das ist aber nur nötig, wenn der Screen im 64column-Mode ist.
		Und das ist fast nie der Fall. Es gibt zu wenige Programme, die das tun.
		Der Einfachheit halber wird deshalb ein Gif mit 256x192 Pixeln erzeugt.
		Im 64column-Mode werden immer 2 Pixel gemischt, was wg. der möglichen Farbkombinationen im 64column-Mode immer
   50% grau ergibt.
*/
void Tc2048GifWriter::drawScreen(
	IoInfo* ioinfo,
	uint	ioinfo_count,
	uint8*	attr_pixels,
	uint	cc_per_scanline,
	uint32	cc_start_of_screenfile,
	bool	flashphase)
{
	assert((cc_start_of_screenfile & 3) == 0);
	if (!bits) bits = new Pixelmap(width, height);

	cc_start_of_screenfile += 4;

	uint32 cc_row_flyback			  = cc_per_scanline - screen_width / pixel_per_cc - 2 * cc_h_border;
	uint32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	uint32 cc_end_of_visible_screen	  = cc_start_of_visible_screen + height * cc_per_scanline - cc_row_flyback;

	// draw border:
	// and screenfile:

	bool	 mode64				= no;
	GifColor mode32_bordercolor = 0;
	GifColor bordercolor		= 0;
	GifColor paper_color		= 0;  // mode64
	GifColor pen_color			= 15; // mode64
	GifColor mixed_color		= 16; // mode64:	immer 50% grau

	uint32 cc_io		   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	GifColor* p			   = bits->getData();							// current pixel pointer
	GifColor* a			   = bits->getData();							// current start of row
	uint	  row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if ((io->addr & 0xfe) != 0xfe) continue; // no ula address

		if (io->cc > cc_start_of_visible_screen)
		{
			uint32 cc = min(cc_end_of_visible_screen, io->cc + 3 & ~3) - cc_start_of_visible_screen;

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
					if (mode64) // 64 column mode
					{
						for (e += screen_width; p < e;)
						{
							uint pixels = *q++;
							for (uint m = 0xC0; m; m = m >> 2)
							{
								*p++ = pixels & m ? ~pixels & m ? mixed_color : pen_color : paper_color;
							}
						}
					}
					else // 32 column mode
					{
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

		cc_io = io->cc;

		if (io->addr & 1) // out(0xff)
		{
			mode64		= io->byte & 4;
			pen_color	= 8 + ((io->byte >> 3) & 7);
			paper_color = pen_color ^ 7;
			if (pen_color == transp) pen_color = 0;
			if (paper_color == transp) paper_color = 0;
			bordercolor = mode64 ? paper_color : mode32_bordercolor;
		}
		else // out(0xfe)
		{
			mode32_bordercolor = io->byte & 7;
			if (!mode64) bordercolor = mode32_bordercolor;
		}
	}

	assert(p == bits->getData() + width * height);
}
