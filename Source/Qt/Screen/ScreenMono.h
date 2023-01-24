#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Screen.h"
#include "IScreenMono.h"

class ScreenMono : public Screen, public IScreenMono
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

	__deprecated bool ffb_or_vbi(uint8* new_pixels,int frame_w,int frame_h,int scrn_w,int scrn_h,int x0,int y0,uint32 cc);
	virtual bool sendFrame(uint8* frame_data, const Size& frame_size, const Rect& screen) override;
};





























