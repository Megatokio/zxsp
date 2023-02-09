// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Libraries/kio/kio.h"
#include "TVDecoderMono.h"

static constexpr float sec_per_scanline = 64e-6f;
static constexpr int32 min_lines_per_frame = 262 - 26;
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
	frame_buffer_size((max_bytes_per_frame / min_bytes_per_line + 1) * (max_bytes_per_line + 1)),
	frame_data(nullptr),
	frame_data2(nullptr),
	frame_size{max_bytes_per_line * 8, max_lines_per_frame},
	background_color(background_color)
{
	frame_data  = new uint8[frame_buffer_size];
	frame_data2 = new uint8[frame_buffer_size];
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
	cc_per_line    = int32(bytes_per_sec*4 * sec_per_scanline);
	cc_per_frame   = bytes_per_sec*4 / 50;
	idx_sync_start = 0;
	idx_line_start = 0;
	idx = 0;
	sync_active = false;
}

void TVDecoderMono::clear_screen_up_to(int new_idx, int byte)
{
	assert(new_idx >= idx && new_idx <= frame_buffer_size);

	memset(frame_data + idx, byte, new_idx - idx);
	idx = new_idx;

	while (new_idx - idx_line_start >= max_bytes_per_line)
	{
		idx_line_start += max_bytes_per_line;
		cc_line_start  += max_bytes_per_line * 4; // cc_per_line;
		current_line += 1;
	}
}

void TVDecoderMono::clear_screen_up_to(int new_idx)
{
	// clear with 'grey' pattern:
	// the pattern alternates between 0xAA and 0x55 between lines

	assert(new_idx <= frame_buffer_size);

	while (idx < new_idx)
	{
		int end = min(new_idx, idx_line_start + max_bytes_per_line);
		int byte = 0x55 << ((idx/max_bytes_per_line) & 1);
		clear_screen_up_to(end, byte);
	}
}

void TVDecoderMono::send_frame(int32 cc)
{
	lines_above_screen = first_screen_line;
	lines_in_screen    = last_screen_line+1 - first_screen_line;
	lines_below_screen = current_line - last_screen_line;
	lines_per_frame    = current_line + 1;

	clear_screen_up_to(max_bytes_per_frame); // grey pattern

	const int top60 = 32;
	const int top50 = 56;
	const int lines60 = 262;
	const int lines50 = 310;

	static_assert(lines50 == lines60 + 2 * 24, "");
	static_assert(top50   == top60   + 1 * 24, "");

	int top = lines_per_frame <= lines60 ? top60 :			// min top60
			  lines_per_frame >= lines50 ? top50 :			// max top50
			  top60 + ((lines_per_frame - lines60) >> 1);	// or interpolate
	int lines = 192;

	zxsp::Rect screen_rect{zxsp::Point{40+32,top},zxsp::Size{256,lines}};

	bool swapped = screen.sendFrame(frame_data,frame_size,screen_rect);
	if (swapped) std::swap(frame_data,frame_data2);

	current_line = 0;
	first_screen_line = 0;
	last_screen_line = 0;

	cc_per_frame    = cc - cc_frame_start;
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
			assert(new_idx <= frame_buffer_size);
		}

		clear_screen_up_to(new_idx, byte);
	}
}

int32 TVDecoderMono::getCcForFrameEnd() const
{
	// return the estimated time for the end of the current/next frame.
	// the machine will run up to this cc and probably overshoot by some cc.

	int32 cc = cc_frame_start + cc_per_frame;
	if (cc < min_bytes_per_frame*4) return min_bytes_per_frame*4;
	if (cc > max_bytes_per_frame*4) return max_bytes_per_frame*4;
	return cc;
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
			if (current_line == first_screen_line+4)
				cc_per_line = cc - cc_line_start;
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

	assert(idx < frame_buffer_size);
	frame_data[idx++] = pixels ^ ~background_color;

	if (unlikely(first_screen_line == 0)) first_screen_line = current_line;
	last_screen_line = max(last_screen_line, current_line);
}

void TVDecoderMono::drawVideoBeamIndicator(int32 cc)
{
	updateScreenUpToCycle(cc);
	//TODO();
}

int32 TVDecoderMono::doFrameFlyback(int32 cc)
{
	// handle vertical frame flyback
	// and return actual duration of last frame.
	// this will be the offset used to shift cc in shiftCcTimeBase()
	// which should reset cc_frame_start back to 0.

	updateScreenUpToCycle(cc);
	//assert(cc_frame_start > 0);
	//return cc_frame_start;

	// have we already seen the next vsync?
	// then in normal cases cc is only very little higher than cc_frame_start.
	if (cc_frame_start > 0) // >= min_bytes_per_frame*4)
	{
		assert(cc_frame_start <= cc);
		return cc_frame_start;
	}

	// we haven't yet seen the next vsync.
	// either the frame duration varies by more than 3cc or the ZX81 is in fast mode.
	// return the highest allowed value: cc must not become negative.
	return cc;
}

void TVDecoderMono::shiftCcTimeBase(int32 cc_delta)
{
	updateScreenUpToCycle(cc_delta);
	cc_frame_start -= cc_delta;
	cc_line_start  -= cc_delta;
}

























