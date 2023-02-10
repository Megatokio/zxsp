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
	cc_per_sec(cc_per_sec),
	typ_cc_per_line(int32(cc_per_sec * sec_per_scanline + 0.5f)),
	min_cc_per_line(int32(cc_per_sec * sec_per_scanline * 0.9f)),
	max_cc_per_line(int32(cc_per_sec * sec_per_scanline * 1.1f)),
	min_cc_for_vsync(int32(cc_per_sec * sec_per_scanline * 2.5f)),
	min_cc_per_frame(max_cc_per_line * min_lines_per_frame),
	max_cc_per_frame(max_cc_per_line * max_lines_per_frame),
	fb_lines(max_cc_per_frame / min_cc_per_line + 2),
	fb_bytes_per_line(((max_cc_per_line+15)>>2) & ~3),
	fb_cc_per_line(fb_bytes_per_line << 2),
	frame_data(nullptr),
	frame_data2(nullptr),
	frame_size{fb_bytes_per_line*8, max_lines_per_frame},
	background_color(background_color),
	foreground_color(~background_color)
{
	frame_data  = new uint8[(max_lines_per_frame+1) * fb_bytes_per_line];
	frame_data2 = new uint8[(max_lines_per_frame+1) * fb_bytes_per_line];
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
	cc_per_line    = typ_cc_per_line;
	cc_per_frame   = cc_per_sec / 50;
	cc_sync_start = 0;
	idx_line_start = 0;
	ccc = 0;
	sync_active = false;
}

static inline uint8 patternX(int line)
{
	// crisscross pattern to mark unset areas in the monitor image

	return 0xAA >> (line & 1);
}

inline void TVDecoderMono::store_pixel_byte(int32 cc, uint8 b)
{
	cc -= cc_line_start;
	int idx = cc >> 2;

	assert(idx >= 0 && idx < fb_bytes_per_line);

	uint8* p = &frame_data[idx_line_start + idx];

	if ((cc & 3) == 0)	// byte aligned :-)
	{
		p[0] = b;
	}
	else				// store across 2 bytes
	{
		int sr   = (cc & 3) * 2;
		int sl   = 8 - sr;
		uint8 mask = uint8(0xff << sl);

		p[0] = (p[0] & mask) | (b >> sr);
		p[1] = uint8(b << sl);		// no need to preserve 'future' pixels
	}
}

void TVDecoderMono::clear_bytes(int32 cca, int32 cce, uint8 pattern)
{
	// fill pixels with pattern
	// the pattern is not shifted for 'odd' cc

	cca -= cc_line_start;
	cce -= cc_line_start;

	assert(cca >= 0 && cca <= cce && cce <= fb_cc_per_line);

	uint8* p = &frame_data[idx_line_start + ( cca    >> 2)];
	uint8* e = &frame_data[idx_line_start + ((cce+3) >> 2)];

	if (cca & 3)
	{
		int sr   = (cca & 3) * 2;
		int mask = 0xff >> sr;
		*p = (*p & ~mask) | (pattern & mask);
		p++;
	}

	while (p < e) { *p++ = pattern; }
}

inline void TVDecoderMono::next_line(int32 cc)
{
	assert(cc >= ccc && cc <= cc_line_start + max_cc_per_line);

	cc_line_start = ccc = cc;
	idx_line_start += fb_bytes_per_line;
	current_line += 1;

	if (current_line >= max_lines_per_frame)
		send_frame(cc);
}

void TVDecoderMono::clear_line_to_end(int32 cc, uint8 pattern)
{
	// clear screen up to line end at cc
	// increments current_line

	assert(cc >= ccc && cc <= cc_line_start + max_cc_per_line);

	clear_bytes(ccc, cc_line_start + fb_cc_per_line, pattern);
	next_line(cc);
}

void TVDecoderMono::clear_screen_to_end()
{
	// clear screen to the end with crisscross patter
	// this does not advance ccc and current_line

	clear_bytes(ccc, cc_line_start + fb_cc_per_line, patternX(current_line));

	int current_line = this->current_line;
	int idx_line_start = this->idx_line_start;

	while (++current_line < max_lines_per_frame)
	{
		idx_line_start += fb_bytes_per_line;
		memset(frame_data + idx_line_start, patternX(current_line), fb_bytes_per_line);
	}
}

void TVDecoderMono::clear_screen_up_to_cc(int32 cc, uint8 pattern)
{
	// clear screen up to cc
	// increments current line if cc >= max_cc_per_line

	while (cc >= cc_line_start + max_cc_per_line)
	{
		clear_line_to_end(cc_line_start + max_cc_per_line, pattern);
	}

	clear_bytes(ccc, cc, pattern);
	ccc = cc;
}

void TVDecoderMono::send_frame(int32 cc)
{
	current_line = 0;
	first_screen_line = 0;
	last_screen_line = 0;

	idx_line_start  = 0;
	//cc_sync_start = 0;
	cc_frame_start  = cc;
	cc_line_start   = cc;
	ccc             = cc;

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
}

int32 TVDecoderMono::getCcForFrameEnd() const
{
	// return the estimated time for the end of the current/next frame.
	// the machine will run up to this cc and probably overshoot by some cc.

	int32 cc = cc_frame_start + cc_per_frame;
	if (cc < min_cc_per_frame) return min_cc_per_frame;
	if (cc > max_cc_per_frame) return max_cc_per_frame;
	return cc;
}

void TVDecoderMono::updateScreenUpToCycle(int32 cc)
{
	if (cc > ccc)
	{
		uint8 pattern = sync_active ? black : background_color;
		clear_screen_up_to_cc(cc, pattern);
	}
}

void TVDecoderMono::syncOn(int32 cc, bool new_state)
{
	if (unlikely(new_state == sync_active)) return;

	updateScreenUpToCycle(cc);
	sync_active = new_state;

	if (new_state)
	{
		cc_sync_start = cc;
	}
	else
	{
		// hsync:
		if (cc - cc_line_start >= min_cc_per_line)
		{
			if (current_line == first_screen_line + 4)		// pick a random in-screen line
				cc_per_line = cc - cc_line_start;			// to update this statistics

			clear_line_to_end(cc, patternX(current_line));	// grey pattern
		}

		// vsync?
		if (cc - cc_sync_start >= min_cc_for_vsync && cc - cc_frame_start >= min_cc_per_frame)
		{
			cc_per_frame	   = cc - cc_frame_start;
			lines_above_screen = first_screen_line;
			lines_in_screen    = last_screen_line+1 - first_screen_line;
			lines_below_screen = current_line - last_screen_line;
			lines_per_frame    = current_line + 1;

			clear_screen_to_end();	// grey pattern
			send_frame(cc);
		}
	}
}

void TVDecoderMono::storePixelByte(int32 cc, uint8 pixels)
{
	if (unlikely(sync_active)) return;
	if (unlikely(cc > ccc || cc >= cc_line_start + max_cc_per_line)) updateScreenUpToCycle(cc);
	store_pixel_byte(cc, pixels ^ foreground_color);
	ccc = cc + 4;

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

	// have we already seen the next vsync?
	// then in normal cases cc is only very little higher than cc_frame_start.
	if (cc_frame_start > 0)
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
	cc_frame_start -= cc_delta;
	cc_line_start  -= cc_delta;
	cc_sync_start  -= cc_delta;
	ccc            -= cc_delta;
}


































