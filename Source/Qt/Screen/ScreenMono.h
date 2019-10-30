/*	Copyright  (c)	Günter Woigk 2008 - 2018
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

#ifndef SCREEN_MONO_H
#define SCREEN_MONO_H

#include "Screen.h"


class ScreenMono : public Screen
{
	// uint16*	_attrpixels;
	// IoInfo*	_ioinfo;
	// uint		_ioinfo_count;
	// uint		_cc_per_scanline;
	// uint32	_cc_start_of_screenfile;
	// bool		_flashphase;
	uint8*	_new_pixels;		// buffer for decoded monochrome video signal
	uint	_frame_w;			// frame width [bytes] == address offset per scan line
	uint	_frame_h;			// frame height [scan line]
	uint	_screen_w;			// screen width [bytes]
	uint	_screen_h;			// screen height [scan lines]
	uint	_screen_x0;			// hor. screen offset inside frame scan lines [bytes]
	uint	_screen_y0;			// vert. screen offset inside frame [scan lines]
	uint32	_cc;

protected:
	//ScreenMono(QWidget*p,isa_id id) :Screen(p,id){}
	void do_ffb_or_vbi() noexcept(false) override; // std::exception
	void paint_screen(bool draw_passepartout=yes) override;

public:
	explicit ScreenMono(QWidget*p)	:Screen(p,isa_ScreenMono){}
	ScreenMono(const ScreenMono&) = delete;
	ScreenMono& operator=(const ScreenMono&) = delete;

	bool ffb_or_vbi(uint8* new_pixels,int frame_w,int frame_h,int scrn_w,int scrn_h,int x0,int y0,uint32 cc);
};


#endif


























