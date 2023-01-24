// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Libraries/kio/kio.h"
#include "TVDecoderMono.h"


static constexpr float sec_per_scanline = 64e-6f;
static constexpr int32 min_lines_per_frame = 240 - 24;
static constexpr int32 max_lines_per_frame = 312 + 32;
static constexpr int32 idx_frame_start = 0;	// always 0

template<typename T>
static inline void memset(void* z, int byte, T size) { memset(z, byte, size_t(size)); }


TVDecoderMono::TVDecoderMono(IScreenMono& screen, int32 cc_per_sec, uint8 background_color) :
	screen(screen),
	bytes_per_sec(cc_per_sec/4),
	min_bytes_per_line(int32(bytes_per_sec * sec_per_scanline * 0.9f)),
	max_bytes_per_line(int32(bytes_per_sec * sec_per_scanline * 1.1f)),
	min_bytes_for_vsync(max_bytes_per_line * 5/2),
	min_bytes_per_frame(max_bytes_per_line * min_lines_per_frame), // note for formula: lines in frame[] are
	max_bytes_per_frame(max_bytes_per_line * max_lines_per_frame), //   always padded to max_bytes_per_line!
	frame_data(nullptr),
	frame_data2(nullptr),
	frame_size{max_bytes_per_line * 8, max_lines_per_frame},
	background_color(background_color)
{
	frame_data  = new uint8[max_bytes_per_frame];
	frame_data2 = new uint8[max_bytes_per_frame];
	reset();
}

TVDecoderMono::~TVDecoderMono()
{
	delete[] frame_data;
	delete[] frame_data2;
}

void TVDecoderMono::reset()
{
	cc_frame_start = 0;
	cc_line_start  = 0;
	idx_sync_start = 0;
	idx_line_start = 0;
	idx = 0;
	sync_active = false;
}

void TVDecoderMono::clear_screen_up_to(int new_idx, int byte)
{
	assert(new_idx >= idx && new_idx <= max_bytes_per_frame);

	memset(frame_data + idx, byte, new_idx - idx);
	idx = new_idx;

	while (new_idx - idx_line_start >= max_bytes_per_line)
	{
		idx_line_start += max_bytes_per_line;
		cc_line_start  += max_bytes_per_line * 4;
	}
}

void TVDecoderMono::clear_screen_up_to(int new_idx)
{
	// clear with 'grey' pattern:
	// the pattern alternates between 0xAA and 0x55 between lines

	assert(new_idx <= max_bytes_per_frame);

	while (idx < new_idx)
	{
		int end = min(new_idx, idx_line_start + max_bytes_per_line);
		int byte = 0x55 << ((idx/max_bytes_per_line) & 1);
		clear_screen_up_to(end, byte);
	}
}

void TVDecoderMono::send_frame(int32 cc)
{
	clear_screen_up_to(max_bytes_per_frame); // grey pattern

	Rect screen_rect{Point{32+40,56},Size{256,192}};

	bool swapped = screen.sendFrame(frame_data,frame_size,screen_rect);
	if (swapped) std::swap(frame_data,frame_data2);

	idx_sync_start  = 0;
	idx_line_start  = 0;
	idx             = 0;
	cc_frame_start  = cc;
	cc_line_start   = cc;
}

void TVDecoderMono::update_screen_up_to(int new_idx)
{
	if (new_idx >= idx)
	{
		uint8 byte = sync_active ? black : background_color;

		if (new_idx >= max_bytes_per_frame)
		{
			clear_screen_up_to(max_bytes_per_frame, byte);
			assert(idx == idx_line_start);
			send_frame(cc_line_start);
			new_idx -= max_bytes_per_frame;
			assert(new_idx <= max_bytes_per_frame);
		}

		clear_screen_up_to(new_idx, byte);
	}
}

void TVDecoderMono::updateScreenUpToCycle(int32 cc)
{
	int new_idx = idx_line_start + (cc-cc_line_start) / 4;
	update_screen_up_to(new_idx);
}

void TVDecoderMono::syncOn(int32 cc, bool new_state)
{
	if (unlikely(new_state == sync_active)) return;
	updateScreenUpToCycle(cc);
	sync_active = new_state;

	if (new_state)
	{
		idx_sync_start = idx;
	}
	else
	{
		// hsync:
		if (idx - idx_line_start >= min_bytes_per_line)
		{
			clear_screen_up_to(idx_line_start + max_bytes_per_line); // grey pattern
			cc_line_start = cc;
		}

		// vsync?
		if (idx - idx_sync_start >= min_bytes_for_vsync && idx - idx_frame_start >= min_bytes_per_frame)
		{
			send_frame(cc);

		}
	}
}

void TVDecoderMono::storePixelByte(int32 cc, uint8 pixels)
{
	if (unlikely(sync_active)) return;

	int new_idx = idx_line_start + (cc-cc_line_start) / 4;
	if (new_idx != idx || idx >= max_bytes_per_frame)
		update_screen_up_to(new_idx);

	assert(uint(idx) < uint(max_bytes_per_frame));
	frame_data[idx++] = pixels ^ ~background_color;
}

void TVDecoderMono::drawVideoBeamIndicator(int32 cc)
{
	updateScreenUpToCycle(cc);
	//TODO();
}

int32 TVDecoderMono::doFrameFlyback(int32 cc)
{
	updateScreenUpToCycle(cc);
	assert(cc_frame_start > 0);
	return cc_frame_start;
}

void TVDecoderMono::shiftCcTimeBase(int32 cc_delta)
{
	updateScreenUpToCycle(cc_delta);
	cc_frame_start -= cc_delta;
	cc_line_start  -= cc_delta;
}



