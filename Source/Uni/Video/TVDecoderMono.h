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
	TVDecoderMono(const TVDecoderMono&)=delete;
	TVDecoderMono& operator=(const TVDecoderMono&)=delete;

	IScreenMono& screen;

	const int32 bytes_per_sec;			// from cc_per_sec
	const int32 min_bytes_per_line;
	const int32 max_bytes_per_line;
	const int32 min_bytes_for_vsync;
	const int32 min_bytes_per_frame;
	const int32 max_bytes_per_frame;

	uint8* frame_data;  	// buffer for decoded monochrome video signal
	uint8* frame_data2; 	// buffer for decoded monochrome video signal

	const Size frame_size;	// width, height of frame data

	int32 cc_frame_start;	// measured from vsync end
	int32 cc_line_start;	// measured from hsync end

	int32 idx_sync_start;
	int32 idx_line_start;	// n * max_bytes_per_line
	int32 idx;

	bool  sync_active;
	uint8 background_color;


public:
	static constexpr uint8 black = 0x00;
	static constexpr uint8 white = 0xff;

	TVDecoderMono(IScreenMono&, int32 cc_per_sec, uint8 background_color=white);
	~TVDecoderMono();

	void reset();				// no need to call
	void setBackgroundColor(uint8 c) { background_color = c; }
	int32 getMaxCyclesPerFrame() const noexcept { return max_bytes_per_frame * 4; }

	void syncOn(int32 cc, bool new_state=true);		// Sync activated at cpu cycle cc
	void syncOff(int32 cc)	{ syncOn(cc,false); }	// Sync deactivated at cpu cycle cc
	void storePixelByte(int32 cc, uint8 pixels);
	void updateScreenUpToCycle(int32 cc);
	int32 doFrameFlyback(int32 cc);	 // and return cc of frame start
	void shiftCcTimeBase(int32 cc_delta);
	void drawVideoBeamIndicator(int32 cc);

private:
	void send_frame(int32 cc);
	void clear_screen_up_to(int new_idx, int byte);
	void clear_screen_up_to(int new_idx);
	void update_screen_up_to(int new_idx);
};

