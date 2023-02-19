// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SpectraRenderer.h"
#include "Templates/Array.h"
#include "cpp/cppthreads.h"
#include "globals.h"
#include "graphics/gif/GifEncoder.h"
#include "unix/os_utilities.h"
#include <QImage>
#include <QPainter>


// Display mode register bit masks:
#define HALFCELLMODE	   0x80
#define ENHANCED_BORDER	   0x10
#define DOUBLE_BYTE_COLOUR 0x08
#define EXTRA_COLOURS	   0x04


#define COLOR(G, R, B) uint32(0xff + ((R)*85 << 24) + ((G)*85 << 16) + ((B)*85 << 8))


const RgbaColor standard_rgba_colors[16] = // RGBA
	{									   // G R B
		COLOR(0, 0, 0), COLOR(0, 0, 2), COLOR(0, 2, 0), COLOR(0, 2, 2), COLOR(2, 0, 0), COLOR(2, 0, 2),
		COLOR(2, 2, 0), COLOR(2, 2, 2), COLOR(0, 0, 0), COLOR(0, 0, 3), COLOR(0, 3, 0), COLOR(0, 3, 3),
		COLOR(3, 0, 0), COLOR(3, 0, 3), COLOR(3, 3, 0), COLOR(3, 3, 3)};

const RgbaColor enhanced_rgba_colors[64] = // RGBA; note: Specci colors are GRB
	{COLOR(0, 0, 0), COLOR(0, 0, 1), COLOR(0, 0, 2), COLOR(0, 0, 3), COLOR(0, 1, 0), COLOR(0, 1, 1),
	 COLOR(0, 1, 2), COLOR(0, 1, 3), COLOR(0, 2, 0), COLOR(0, 2, 1), COLOR(0, 2, 2), COLOR(0, 2, 3),
	 COLOR(0, 3, 0), COLOR(0, 3, 1), COLOR(0, 3, 2), COLOR(0, 3, 3),

	 COLOR(1, 0, 0), COLOR(1, 0, 1), COLOR(1, 0, 2), COLOR(1, 0, 3), COLOR(1, 1, 0), COLOR(1, 1, 1),
	 COLOR(1, 1, 2), COLOR(1, 1, 3), COLOR(1, 2, 0), COLOR(1, 2, 1), COLOR(1, 2, 2), COLOR(1, 2, 3),
	 COLOR(1, 3, 0), COLOR(1, 3, 1), COLOR(1, 3, 2), COLOR(1, 3, 3),

	 COLOR(2, 0, 0), COLOR(2, 0, 1), COLOR(2, 0, 2), COLOR(2, 0, 3), COLOR(2, 1, 0), COLOR(2, 1, 1),
	 COLOR(2, 1, 2), COLOR(2, 1, 3), COLOR(2, 2, 0), COLOR(2, 2, 1), COLOR(2, 2, 2), COLOR(2, 2, 3),
	 COLOR(2, 3, 0), COLOR(2, 3, 1), COLOR(2, 3, 2), COLOR(2, 3, 3),

	 COLOR(3, 0, 0), COLOR(3, 0, 1), COLOR(3, 0, 2), COLOR(3, 0, 3), COLOR(3, 1, 0), COLOR(3, 1, 1),
	 COLOR(3, 1, 2), COLOR(3, 1, 3), COLOR(3, 2, 0), COLOR(3, 2, 1), COLOR(3, 2, 2), COLOR(3, 2, 3),
	 COLOR(3, 3, 0), COLOR(3, 3, 1), COLOR(3, 3, 2), COLOR(3, 3, 3)};


inline RgbaColor rgbaColorForBorderbyte(uint8 b)
{
	return COLOR(((b >> 1) & 2) + ((b >> 7) & 1), ((b >> 0) & 2) + ((b >> 6) & 1), ((b << 1) & 2) + ((b >> 5) & 1));
}


/*	rendere Ausgaben der Zxsp Ula in bits[].

	ioinfo[]: alle OUTs zur ULA
		-> set Border
		ioinfo[] muss mit einem OUT anfangen, das die Bordercolor setzt!
		ioinfo[] muss am Ende noch einen freien Platz haben!

	attr_pixels[192*32]: von der ULA ausgegebene Attribut/Pixel-Pärchen
		pro Scanline werden 32 Pärchen (64 Bytes) ausgegeben
*/
void SpectraRenderer::drawScreen(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase, uint32 cc_vbi)
{
	assert((cc_start_of_screenfile & 3) == 0);

	cc_start_of_screenfile += 4;
	cc_vbi = (cc_vbi + 3) & ~3;

	uint32 cc_row_flyback			  = cc_per_scanline - cc_screen - 2 * cc_h_border;
	uint32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	if (cc_vbi < cc_start_of_visible_screen) return; // video beam ist noch über dem sichtbaren Bildschirmausschnitt
	uint32 cc_end_of_visible_screen =
		min(cc_vbi, cc_start_of_visible_screen + height * cc_per_scanline - cc_row_flyback);

	// draw border and screenfile:

	RgbaColor bordercolor = black;

	uint8 borderbyte = 0;
	uint8 video_mode = 0;

	uint32 cc_io		   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	RgbaColor* p		   = bits;										// current pixel pointer
	RgbaColor* a		   = bits;										// current start of row
	uint	   row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if (io->cc > cc_start_of_visible_screen)
		{
			uint32 cc = min(cc_end_of_visible_screen, io->cc + 3 & ~3) - cc_start_of_visible_screen;

			uint end_row = cc / cc_per_scanline;
			uint end_col = min(uint(width), cc % cc_per_scanline * pixel_per_cc);
			assert(end_row < height);
			RgbaColor* ee = bits + end_row * width + end_col; // cc_io end pointer
			RgbaColor* e;									  // intermediate ent pointers

			// draw all border pixels up to cc_io:
			while (p < ee)
			{
				if (row >= v_border && row < v_border + 192) // if inside screenfile region:
				{
					e = min(a + h_border, ee);
					while (p < e) { *p++ = bordercolor; } // draw left border
					if (p < a + h_border) break;		  // exit pixel loop if at cc_io

					// draw screen row:
					e = min(ee, e + screen_width);
					while (p < e)
					{
						uint pixels = *q++;
						uint attr1	= *q++;
						uint attr2	= *q++;
						uint m		= 0x80;

						RgbaColor color1, color2, color3, color4;

						if (video_mode & DOUBLE_BYTE_COLOUR)
						{
							if (flashphase)
							{
								if (attr1 & 0x80)
								{
									if (attr2 & 0x80) pixels ^= 0xff;
									else pixels = 0x00;
								}
								else if (attr2 & 0x80) pixels = 0xff;
							}

							if (video_mode & EXTRA_COLOURS) // 2-byte attr, extra colors
							{
								color1 = enhanced_rgba_colors[attr1 & 0x3F]; // fullcell: pen; halfcell: penR
								color2 = enhanced_rgba_colors[attr2 & 0x3F]; // fullcell: pap; halfcell: penL
								//															  halfcell: pap=black  ((future: papL
								//and papR may be black or white))

								if (video_mode & HALFCELLMODE)
								{
									for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color2 : black; }
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : black; }
								}
								else
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
							}
							else // 2-byte attr, standard colors
							{
								color1 = standard_rgba_colors[(attr1 & 7) + ((attr1 >> 3) & 8)]; // fullcell: pen;
																								 // halfcell: penR
								color2 = standard_rgba_colors[(attr2 & 7) + ((attr2 >> 3) & 8)]; // fullcell: pap;
																								 // halfcell: papR
								color3 = standard_rgba_colors[(attr1 >> 3) & 15];				 //				  halfcell: penL
								color4 = standard_rgba_colors[(attr2 >> 3) & 15];				 //				  halfcell: papL

								if (video_mode & HALFCELLMODE)
								{
									for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color3 : color4; }
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
								}
								else
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
							}
						}
						else // 1-byte attr
						{
							if (attr1 & 0x80 && flashphase) pixels ^= 0xff;

							if (video_mode & EXTRA_COLOURS)					 // 1-byte attr, extra colors
							{												 //				  halfcell: pap=black
								color1 = enhanced_rgba_colors[attr1 & 0x3F]; // fullcell: pen; halfcell: penR
								color2 =
									attr1 & 0x40 ? standard_rgba_colors[7] : black; // fullcell: pap; halfcell: penL
							}
							else // 1-byte attr, standard colors
							{	 //				  halfcell: pap=black
								color1 = standard_rgba_colors[(attr1 & 7) + ((attr1 >> 3) & 8)]; // fullcell: pen;
																								 // halfcell: penR
								color2 = standard_rgba_colors[(attr1 >> 3) & 15]; // fullcell: pap; halfcell: penL
							}

							if (video_mode & HALFCELLMODE)
							{
								for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color2 : black; }
								for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : black; }
							}
							else
								for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
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

		if (~io->addr & 1) borderbyte = io->byte;
		else if (io->addr == 0x7FDF) video_mode = io->byte;

		bordercolor = video_mode & ENHANCED_BORDER // enhanced border?
						  ?
						  video_mode & EXTRA_COLOURS //  enhanced border: extra colours?
							  ?
						  rgbaColorForBorderbyte(borderbyte) //   enhanced border, extra colours
						  :
						  (borderbyte & 0x80) && flashphase //   enhanced border, basic colours: flash?
								  ?
						  borderbyte & 0x20 //    flash: flash with black or white?
									  ?
						  borderbyte & 0x40 //     flash with white: dimm or bright?
										  ?
						  COLOR(3, 3, 3) /*bright white*/ //      bright
						  :
						  COLOR(2, 2, 2) /*dimm white*/ //      dimm
						  :
						  COLOR(0, 0, 0) /*black*/ //     flash with black
						  :
						  standard_rgba_colors[(borderbyte & 7) + ((borderbyte >> 3) & 8)] //    not flashing: 8 basic
																						   //    colours bright or dimm
						  :
						  standard_rgba_colors[borderbyte & 7]; //  standard border
	}

	if (p < bits + width * height) // Video beam indicator
	{
		assert(p <= bits + width * height - 8);
		RgbaColor c = int(system_time * 6) & 1 ? bright_yellow : bright_red;
		for (uint i = 0; i < 8; i++) p[i] = c;
		return;
	}

	assert(p == bits + width * height);
}


// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================


#undef COLOR
#define COLOR(G, R, B) R * 85, G * 85, B * 85

const Comp spectra_colors[65 * 3] = // R,G,B,		note: Specci colors are GRB
	{
		// G,R,B
		COLOR(0, 0, 0), COLOR(0, 0, 1), COLOR(0, 0, 2), COLOR(0, 0, 3), COLOR(0, 1, 0), COLOR(0, 1, 1),
		COLOR(0, 1, 2), COLOR(0, 1, 3), COLOR(0, 2, 0), COLOR(0, 2, 1), COLOR(0, 2, 2), COLOR(0, 2, 3),
		COLOR(0, 3, 0), COLOR(0, 3, 1), COLOR(0, 3, 2), COLOR(0, 3, 3),

		COLOR(1, 0, 0), COLOR(1, 0, 1), COLOR(1, 0, 2), COLOR(1, 0, 3), COLOR(1, 1, 0), COLOR(1, 1, 1),
		COLOR(1, 1, 2), COLOR(1, 1, 3), COLOR(1, 2, 0), COLOR(1, 2, 1), COLOR(1, 2, 2), COLOR(1, 2, 3),
		COLOR(1, 3, 0), COLOR(1, 3, 1), COLOR(1, 3, 2), COLOR(1, 3, 3),

		COLOR(2, 0, 0), COLOR(2, 0, 1), COLOR(2, 0, 2), COLOR(2, 0, 3), COLOR(2, 1, 0), COLOR(2, 1, 1),
		COLOR(2, 1, 2), COLOR(2, 1, 3), COLOR(2, 2, 0), COLOR(2, 2, 1), COLOR(2, 2, 2), COLOR(2, 2, 3),
		COLOR(2, 3, 0), COLOR(2, 3, 1), COLOR(2, 3, 2), COLOR(2, 3, 3),

		COLOR(3, 0, 0), COLOR(3, 0, 1), COLOR(3, 0, 2), COLOR(3, 0, 3), COLOR(3, 1, 0), COLOR(3, 1, 1),
		COLOR(3, 1, 2), COLOR(3, 1, 3), COLOR(3, 2, 0), COLOR(3, 2, 1), COLOR(3, 2, 2), COLOR(3, 2, 3),
		COLOR(3, 3, 0), COLOR(3, 3, 1), COLOR(3, 3, 2), COLOR(3, 3, 3), COLOR(0, 0, 0) // transp
};

using GifColor = uint8;

const GifColor transp = 64;
const Colormap spectra_colormap(spectra_colors, 65, transp);

#define B	 2
#define R	 8
#define G	 32
#define H(C) (C) / 2 * 3

// conversion table: basic color [0..15] -> index in spectra_colors[]
GifColor basic_colors[16] = {
	0,		   // black
	B,		   // blue
	R,		   // red
	R + B,	   // magenta
	G,		   // green
	G + B,	   // cyan
	G + R,	   // yellow
	G + R + B, // dimm white

	H(0),		 // black
	H(B),		 // blue
	H(R),		 // red
	H(R + B),	 // magenta
	H(G),		 // green
	H(G + B),	 // cyan
	H(G + R),	 // yellow
	H(G + R + B) // dimm white
};

#undef R
#undef G
#undef B
#undef H

inline GifColor gifColorForBorderbyte(uint8 b)
{
	return ((b & 128) >> 3) + ((b & 64) >> 4) + ((b & 32) >> 5) + ((b & 4) << 3) + ((b & 2) << 2) + ((b & 1) << 1);
}


SpectraGifWriter::SpectraGifWriter(QObject* p, bool update_border, uint frames_per_second) :
	ZxspGifWriter(p, isa_SpectraGifWriter, spectra_colormap, update_border, frames_per_second)
{}


void SpectraGifWriter::drawScreen(
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

	GifColor bordercolor = 0; // black

	uint8 borderbyte = 0;
	uint8 video_mode = 0;

	uint32 cc_io		   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	GifColor* p			   = bits->getData();							// current pixel pointer
	GifColor* a			   = bits->getData();							// current start of row
	uint	  row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
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
				if (row >= v_border && row < v_border + 192) // if inside screenfile region:
				{
					e = min(a + h_border, ee);
					while (p < e) { *p++ = bordercolor; } // draw left border
					if (p < a + h_border) break;		  // exit pixel loop if at cc_io

					// draw screen row:
					for (e += screen_width; p < e;)
					{
						uint pixels = *q++;
						uint attr1	= *q++;
						uint attr2	= *q++;
						uint m		= 0x80;

						RgbaColor color1, color2, color3, color4;

						if (video_mode & DOUBLE_BYTE_COLOUR)
						{
							if (flashphase)
							{
								if (attr1 & 0x80)
								{
									if (attr2 & 0x80) pixels ^= 0xff;
									else pixels = 0x00;
								}
								else if (attr2 & 0x80) pixels = 0xff;
							}

							if (video_mode & EXTRA_COLOURS) // 2-byte attr, extra colors
							{
								color1 = attr1 & 0x3F; // fullcell: pen; halfcell: penR
								color2 = attr2 & 0x3F; // fullcell: pap; halfcell: penL
								//										  halfcell: pap=black  ((future: papL and papR
								//may be black or white))

								if (video_mode & HALFCELLMODE)
								{
									for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color2 : 0 /*black*/; }
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : 0 /*black*/; }
								}
								else
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
							}
							else // 2-byte attr, standard colors
							{
								color1 =
									basic_colors[(attr1 & 7) + ((attr1 >> 3) & 8)]; // fullcell: pen; halfcell: penR
								color2 =
									basic_colors[(attr2 & 7) + ((attr2 >> 3) & 8)]; // fullcell: pap; halfcell: papR
								color3 = basic_colors[(attr1 >> 3) & 15];			//				  halfcell: penL
								color4 = basic_colors[(attr2 >> 3) & 15];			//				  halfcell: papL

								if (video_mode & HALFCELLMODE)
								{
									for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color3 : color4; }
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
								}
								else
									for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
							}
						}
						else // 1-byte attr
						{
							if (attr1 & 0x80 && flashphase) pixels ^= 0xff;

							if (video_mode & EXTRA_COLOURS)							   // 1-byte attr, extra colors
							{														   //				  halfcell: pap=black
								color1 = attr1 & 0x3F;								   // fullcell: pen; halfcell: penR
								color2 = attr1 & 0x40 ? basic_colors[7] : 0 /*black*/; // fullcell: pap; halfcell: penL
							}
							else // 1-byte attr, standard colors
							{	 //				  halfcell: pap=black
								color1 =
									basic_colors[(attr1 & 7) + ((attr1 >> 3) & 8)]; // fullcell: pen; halfcell: penR
								color2 = basic_colors[(attr1 >> 3) & 15];			// fullcell: pap; halfcell: penL
							}

							if (video_mode & HALFCELLMODE)
							{
								for (; m > 15; m = m >> 1) { *p++ = pixels & m ? color2 : 0 /*black*/; }
								for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : 0 /*black*/; }
							}
							else
								for (; m; m = m >> 1) { *p++ = pixels & m ? color1 : color2; }
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

		if (~io->addr & 1) borderbyte = io->byte;
		else if (io->addr == 0x7FDF) video_mode = io->byte;

		bordercolor = video_mode & ENHANCED_BORDER // enhanced border?
						  ?
						  video_mode & EXTRA_COLOURS //  enhanced border: extra colours?
							  ?
						  gifColorForBorderbyte(borderbyte) //   enhanced border, extra colours
						  :
						  (borderbyte & 0x80) && flashphase //   enhanced border, basic colours: flash?
								  ?
						  borderbyte & 0x20 //    flash: flash with black or white?
									  ?
						  borderbyte & 0x40 //     flash with white: dimm or bright?
										  ?
						  63 /*bright white*/ //      bright
						  :
						  42 /*dimm white*/ //      dimm
						  :
						  0 /*black*/ //     flash with black
						  :
						  basic_colors[(borderbyte & 7) + ((borderbyte >> 3) & 8)] //    not flashing: 8 basic colours
																				   //    bright or dimm
						  :
						  basic_colors[borderbyte & 7]; //  standard border
	}

	assert(p == bits->getData() + width * height);
}
