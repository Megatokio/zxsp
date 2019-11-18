#pragma once
/*	Copyright  (c)	Günter Woigk 2008 - 2019
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





























