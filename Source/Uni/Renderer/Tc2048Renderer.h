/*	Copyright  (c)	Günter Woigk 2013 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/


#ifndef TC2048RENDERER_H
#define TC2048RENDERER_H

#include "ZxspRenderer.h"


// ===========================================================
//					screen renderer:
// ===========================================================


class Tc2048Renderer : public ZxspRenderer
{
public:
	static const int
		h_border	 = 64*2,			// pixel, must be N*8
		v_border	 = 48,				// pixel, must be N*8
		screen_width = 256*2,
		screen_height= 192,
		width		 = screen_width+2*h_border,		// width of bits[]
		height		 = screen_height+2*v_border,	// height of bits[]

		pixel_per_cc = 2*2,

		cc_screen	 = screen_width/pixel_per_cc,	// 128 -> 256 pixel
		cc_h_border	 = h_border/pixel_per_cc;		// 32  -> 64 pixel


	explicit Tc2048Renderer(QObject*p)
					:ZxspRenderer(p,isa_Tc2048Renderer,screen_width,screen_height,h_border,v_border){}

	void drawScreen( IoInfo*, uint ioinfo_count, uint8 *attr_pixels, uint cc_per_scanline,
					 uint32 cc_start_of_screenfile, bool flashphase, uint32 cc_vbi) override;
};


// ===========================================================
//					gif file creation:
// ===========================================================


class Tc2048GifWriter : public ZxspGifWriter
{
public:
	Tc2048GifWriter( QObject* p, bool update_border );

	void drawScreen( IoInfo*, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline,
					 uint32 cc_start_of_screenfile, bool flashphase) override;
};


#endif




















