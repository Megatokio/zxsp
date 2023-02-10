// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include "IScreenMono.h"

/*
	This class accepts callbacks related to creating a video signal
	and creates the video image.
	It is used for ZX80 and ZX81 video image.
	The image is b&w only.
	The TV image synchronizes to approx. 45 to 65 Hz.
 */

class TVDecoderMono
{
	NO_COPY_MOVE(TVDecoderMono);

	IScreenMono& screen;

	const int32 cc_per_sec;
	const int32 typ_cc_per_line;
	const int32 min_cc_per_line;
	const int32 max_cc_per_line;
	const int32 min_cc_for_vsync;
	const int32 min_cc_per_frame;
	const int32 max_cc_per_frame;
	const int32 fb_lines;
	const int32 fb_bytes_per_line;
	const int32 fb_cc_per_line;

	uint8* frame_data;  	// buffer for decoded monochrome video signal
	uint8* frame_data2; 	// buffer for decoded monochrome video signal

	const zxsp::Size frame_size; // width, height of frame data

	int32 cc_frame_start;	// measured from vsync end
	int32 cc_line_start;	// measured from hsync end
	int32 cc_per_frame; 	// duration of last successful frame
	int32 cc_per_line;		// duration of last successful line

	int32 cc_sync_start;
	int32 idx_line_start;	// n * max_bytes_per_line
	int32 ccc;

	bool  sync_active;
	uint8 background_color;
	uint8 foreground_color;

	int	current_line = 0;			// for statistics
	int first_screen_line = 0;
	int last_screen_line = 0;

public:
	int lines_above_screen = 56;	// statistics output for Ula Inspector
	int lines_in_screen = 192;
	int lines_below_screen = 62;
	int lines_per_frame = 310;

	static constexpr uint8 black = 0x00;
	static constexpr uint8 white = 0xff;

	TVDecoderMono(IScreenMono&, int32 cc_per_sec, uint8 background_color = white);
	~TVDecoderMono();

	void reset();				// no need to call
	void setBackgroundColor(uint8 c) { background_color = c; foreground_color = ~c; }
	int32 getMaxCyclesPerFrame() const noexcept { return max_cc_per_frame; }

	void syncOn(int32 cc, bool new_state=true);		// Sync activated at cpu cycle cc
	void syncOff(int32 cc)	{ syncOn(cc,false); }	// Sync deactivated at cpu cycle cc
	void storePixelByte(int32 cc, uint8 pixels);
	void updateScreenUpToCycle(int32 cc);
	int32 doFrameFlyback(int32 cc);	 // and return cc of frame start
	void shiftCcTimeBase(int32 cc_delta);
	void drawVideoBeamIndicator(int32 cc);
	int32 getCcPerLine() volatile const			{ return cc_per_line; }
	int32 getCcPerFrame() volatile const		{ return cc_per_frame; }
	int32 getCycleOfFrameStart() volatile const	{ return cc_frame_start; }
	int32 getCcForFrameEnd() const;

private:
	void store_pixel_byte(int32,uint8);
	void clear_bytes(int32, int32, uint8 pattern);
	void next_line(int32);
	void clear_line_to_end(int32 cc, uint8 pattern);
	void clear_screen_to_end();
	void clear_screen_up_to_cc(int32 new_cc, uint8 byte);
	void send_frame(int32 cc);
};


