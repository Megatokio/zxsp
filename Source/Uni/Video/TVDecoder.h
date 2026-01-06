// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Items/Ula/Crtc.h"
#include "VideoData.h"

/*
	This class accepts callbacks related to creating a video signal
	and creates the video image.
	It is used for ZX80 and ZX81 video image.
	The image may have colour to support the CHROMA81 interface.
	The TV image synchronizes to approx. 45 to 65 Hz.
*/

namespace zxsp
{

class TVDecoder
{
	NO_COPY_MOVE(TVDecoder);

public:
	static constexpr int min_lines_per_frame = 262 - 26;
	static constexpr int max_lines_per_frame = 312 + 32;

	static constexpr uint8 black		  = 0;	// IGRB
	static constexpr uint8 white		  = 15; // IGRB
	static constexpr uint8 white_ink	  = white;
	static constexpr uint8 black_ink	  = black;
	static constexpr uint8 white_paper	  = white << 4;
	static constexpr uint8 black_paper	  = black << 4;
	static constexpr uint8 black_on_white = black_ink + white_paper;

	int lines_above_screen = 56; // for display in Ula Inspector
	int lines_in_screen	   = 192;
	int lines_below_screen = 62;
	int lines_per_frame	   = 310;

	const int32 cc_per_sec;
	const int32 typ_cc_per_line;
	const int32 min_cc_per_line;
	const int32 max_cc_per_line;
	const int32 min_cc_for_vsync;
	const int32 min_cc_per_frame;
	const int32 max_cc_per_frame;
	//const int32 fb_lines;
	const int32 fb_bytes_per_line;
	const int32 fb_cc_per_line;

	TVDecoder(Crtc* crtc, int32 cc_per_sec, uint8 background_color = white);
	~TVDecoder();

	int32 getMaxCyclesPerFrame() const noexcept { return max_cc_per_frame; }

	void  syncOn(int32 cc, bool new_state = true); // Sync activated at cpu cycle cc
	void  syncOff(int32 cc) { syncOn(cc, false); } // Sync deactivated at cpu cycle cc
	void  storePixelByte(int32 cc, uint8 pixels, uint8 attr = black_on_white);
	void  updateScreenUpToCycle(int32 cc);
	int32 doFrameFlyback(int32 cc); // and return cc of frame start
	void  shiftCcTimeBase(int32 cc_delta);
	void  drawVideoBeamIndicator(int32 cc);
	int32 getCcPerLine() const volatile { return cc_per_line; }
	int32 getCcPerFrame() const volatile { return cc_per_frame; }
	int32 getCycleOfFrameStart() const volatile { return cc_frame_start; }
	int32 getCcForFrameEnd() const;
	void  reset(int32 cc);
	void  setBorderColor(int32 cc, uint8 b);

private:
	Crtc* crtc;

	Zx80VideoData* bucket = nullptr;

	uint8 border_attr = white_paper;
	bool  sync_active = no;

	zxsp::Point screen_position {40 + 32, 32};
	int			cc_pixel_offset; // offset 0 .. 3 to align screen to byte boundary

	int32 cc_frame_start; // measured from vsync end
	int32 cc_line_start;  // measured from hsync end
	int32 cc_sync_start;  //
	int32 cc_per_frame;	  // duration of last successful frame
	int32 cc_per_line;	  // duration of last successful line
	int32 ccc;			  // current / last cc

	int32 idx_line_start; // n * max_bytes_per_line

	int current_line;
	int first_screen_line;
	int last_screen_line;

	// auto positioning:
	uint8 cc_left[max_lines_per_frame + 1];	 // collect data
	uint8 cc_right[max_lines_per_frame + 1]; // collect data
	Point new_screen_position {40 + 32, 32};
	int	  new_cc_pixel_offset	  = 0;
	int	  auto_position_countdown = 0;

	void store_pixels(int32 cc, uint8 pixels, uint8 attr);
	void clear_pixels(int32 cca, int32 cce, uint8 attr);
	void clear_screen_up_to_cc(int32 cc, uint8 attr);
	void next_line(int32 cc);
	void send_frame(int32 cc);
	void auto_position_screen();
	void update_right_border_info(int line, int32 cc);
	void update_left_border_info(int line, int32 cc);
	void reset_auto_position_data();
};

} // namespace zxsp

/*









































*/
