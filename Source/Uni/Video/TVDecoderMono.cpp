// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TVDecoderMono.h"
#include "Libraries/kio/kio.h"

static constexpr float sec_per_scanline = 64e-6f;

template<typename T>
static inline void memset(void* z, int byte, T size)
{
	memset(z, byte, size_t(size));
}


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
	fb_bytes_per_line(((max_cc_per_line + 15) >> 2) & ~3),
	fb_cc_per_line(fb_bytes_per_line << 2),
	frame_data(nullptr),
	frame_data2(nullptr),
	background_color(background_color),
	foreground_color(~background_color)
{
	frame_data	= new uint8[(max_lines_per_frame + 1) * fb_bytes_per_line];
	frame_data2 = new uint8[(max_lines_per_frame + 1) * fb_bytes_per_line];
	reset();
}

TVDecoderMono::~TVDecoderMono()
{
	delete[] frame_data;
	delete[] frame_data2;
}

void TVDecoderMono::reset()
{
	cc_pixel_offset			= 0;
	cc_frame_start			= 0;
	cc_line_start			= 0;
	cc_per_line				= typ_cc_per_line;
	cc_per_frame			= cc_per_sec / 50;
	cc_sync_start			= 0;
	idx_line_start			= 0;
	ccc						= 0;
	sync_active				= false;
	current_line			= 0;
	auto_position_countdown = 0;
	reset_auto_position_data();
}

void TVDecoderMono::reset_auto_position_data()
{
	first_screen_line = 0;
	last_screen_line  = 0;
	memset(cc_right, 0, sizeof(cc_right));
	memset(cc_left, 255, sizeof(cc_left));
}

inline void TVDecoderMono::store_pixels(int32 cc, uint8 pixels)
{
	// store 8 pixels

	cc -= cc_line_start;
	int idx = cc >> 2;

	assert(idx >= 0 && idx < fb_bytes_per_line);

	uint8* p = &frame_data[idx_line_start + idx];

	if ((cc & 3) == 0) // byte aligned :-)
	{
		p[0] = pixels;
	}
	else // store across 2 bytes
	{
		int	  sr   = (cc & 3) * 2;
		int	  sl   = 8 - sr;
		uint8 mask = uint8(0xff << sl);

		p[0] = (p[0] & mask) | (pixels >> sr);
		p[1] = uint8(pixels << sl); // no need to preserve 'future' pixels
	}
}

void TVDecoderMono::clear_pixels(int32 cca, int32 cce, uint8 color)
{
	// clear pixels with color

	cca -= cc_line_start;
	cce -= cc_line_start;

	assert(cca >= 0 && cca <= cce && cce <= fb_cc_per_line);

	uint8* p = &frame_data[idx_line_start + (cca >> 2)];
	uint8* e = &frame_data[idx_line_start + ((cce + 3) >> 2)];

	if (cca & 3)
	{
		int sr	 = (cca & 3) * 2;
		int mask = 0xff >> sr;
		*p		 = (*p & ~mask) | (color & mask);
		p++;
	}

	while (p < e) { *p++ = color; }
}

inline void TVDecoderMono::next_line(int32 cc)
{
	assert(cc >= ccc && cc <= cc_line_start + max_cc_per_line);

	cc_line_start = ccc = cc;
	idx_line_start += fb_bytes_per_line;
	current_line += 1;

	if (current_line >= max_lines_per_frame) send_frame(cc);
}

void TVDecoderMono::clear_screen_up_to_cc(int32 cc, uint8 color)
{
	// clear screen up to cc with color
	// increments current_line if cc >= max_cc_per_line
	// sends a frame if current_line >= max_lines_per_frame

	if (cc > ccc)
	{
		while (cc >= cc_line_start + max_cc_per_line)
		{
			clear_pixels(ccc, cc_line_start + fb_cc_per_line, color);
			next_line(cc_line_start + max_cc_per_line);
		}

		clear_pixels(ccc, cc, color);
		ccc = cc;
	}
}

void TVDecoderMono::auto_position_screen()
{
	// calculate enclosing rectangle for all screen pixels

	int top	   = first_screen_line;
	int bottom = last_screen_line + 1;
	int height = bottom - top;

	if (top < 10) return;
	if (bottom > current_line - 10) return;
	if (height < 8) return;
	if (height > 240) return;

	int left  = 255;
	int right = 0;
	for (int i = top + 4; i < bottom; i += 8)
	{
		left  = min(left, int(cc_left[i]));
		right = max(right, int(cc_right[i]));
	}
	int width = right - left; // all in cc, not pixels!

	while (width > 40 * 4)
	{
		// new screen is much too wide
		// try to exclude extremely long lines:

		int l = 255, r = 0;
		for (int row = top + 4; row < bottom; row += 8)
		{
			int z = cc_left[row];
			if (z < l && z > left) l = z;
			z = cc_right[row];
			if (z > r && z < right) r = z;
		}
		if (l != 255) left = l;
		if (r != 0) right = r;
		if (width == right - left) return;
		width = right - left;
	}

	// desired vertical position:

	const int top60	  = 32;
	const int top50	  = top60 + 1 * 24; // 56
	const int lines60 = 262;
	const int lines50 = lines60 + 2 * 24; // 310

	int top0 = lines_per_frame <= lines60 ? top60 :			// min top60
			   lines_per_frame >= lines50 ? top50 :			// max top50
			   top60 + ((lines_per_frame - lines60) >> 1);	// interpolate

	if (height > 192) // larger than standard screen
	{
		// top0 = top + (height-192)/2;	// centered
		if (top > top0) top0 = top;
		if (bottom < top0 + 192) top0 = bottom - 192;
		if (top < top0 - 24) top0 = top + 24;
		if (bottom > top0 + 192 + 24) top0 = bottom - 192 - 24;
	}
	else
	{
		if (top < top0) top0 = top;
		if (bottom > top0 + 192) top0 = bottom - 192;
	}

	// desired horizontal position:

	left -= cc_pixel_offset;   // cc
	right -= cc_pixel_offset;  // cc
	int left0 = (40 + 40) / 2; // cc

	if (width > 128) // larger than standard screen
	{
		left0 = left + (width - 128) / 2; // centered
	}
	else
	{
		if (left < left0) left0 = left;
		if (right > left0 + 128) left0 = right - 128;
	}
	int cc_offset = -left0 & 3;
	left0 += cc_offset;
	assert((left0 & 3) == 0);
	left0 *= 2;

	// after some delay, slowly move to new position:

	if (new_screen_position.x == left0 && new_screen_position.y == top0 && new_cc_pixel_offset == cc_offset)
	{
		if (++auto_position_countdown >= 50)
		{
			cc_pixel_offset	  = cc_offset;
			screen_position.x = left0;
			screen_position.y += sign(top0 - screen_position.y);
			if (screen_position.y == top0) auto_position_countdown = INT_MIN;
		}
	}
	else
	{
		new_cc_pixel_offset		= cc_offset;
		new_screen_position.x	= left0;
		new_screen_position.y	= top0;
		auto_position_countdown = 0;
	}
}

void TVDecoderMono::send_frame(int32 cc)
{
	auto_position_screen();

	zxsp::Size frame_size {fb_bytes_per_line << 3, max_lines_per_frame};
	zxsp::Rect screen_rect {screen_position, zxsp::Size {256, 192}};

	bool swapped = screen.sendFrame(frame_data, frame_size, screen_rect);
	if (swapped) std::swap(frame_data, frame_data2);

	current_line   = 0;
	idx_line_start = 0;
	// cc_sync_start = 0;
	cc_frame_start = cc;
	cc_line_start  = cc;
	ccc			   = cc;

	reset_auto_position_data();
}

void TVDecoderMono::update_right_border_info(int line, int32 cc)
{
	// called at hsync on

	cc -= cc_line_start;
	assert(cc == uint8(cc));
	cc_right[line] = uint8(cc);
}

void TVDecoderMono::update_left_border_info(int line, int32 cc)
{
	// called at store_pixel_byte

	if (unlikely(first_screen_line == 0)) first_screen_line = line;
	last_screen_line = line;

	cc -= cc_line_start;
	assert(cc == uint8(cc));
	cc_left[line] = uint8(cc);
}

void TVDecoderMono::syncOn(int32 cc, bool new_state)
{
	if (unlikely(new_state == sync_active)) return;

	if (new_state == on)
	{
		update_right_border_info(current_line, ccc);
		clear_screen_up_to_cc(cc, background_color);
		sync_active	  = on;
		cc_sync_start = cc;
	}
	else
	{
		clear_screen_up_to_cc(cc, black);
		sync_active = off;

		// vsync?
		if (cc - cc_sync_start >= min_cc_for_vsync && cc - cc_frame_start >= min_cc_per_frame)
		{
			clear_screen_up_to_cc(cc_line_start + max_cc_per_line, black);

			cc_per_frame	   = cc - cc_frame_start;
			lines_per_frame	   = current_line;
			lines_above_screen = first_screen_line;
			lines_in_screen	   = last_screen_line + 1 - first_screen_line;
			lines_below_screen = lines_per_frame - (last_screen_line + 1);

			memset(frame_data + idx_line_start, black, max_lines_per_frame * fb_bytes_per_line - idx_line_start);
			send_frame(cc);
			clear_screen_up_to_cc(cc + 16, black); // back porch
		}

		// hsync?
		else if (cc - cc_line_start >= min_cc_per_line)
		{
			cc_per_line = cc - cc_line_start; // for UlaInsp
			clear_pixels(ccc, cc_line_start + max_cc_per_line, black);
			next_line(cc);
			clear_screen_up_to_cc(cc + 16, black); // back porch
		}
	}
}

void TVDecoderMono::storePixelByte(int32 cc, uint8 pixels)
{
	if (unlikely(sync_active)) return;

	cc += cc_pixel_offset;

	if (unlikely(cc != ccc || cc >= cc_line_start + max_cc_per_line))
	{
		clear_screen_up_to_cc(cc, background_color);
		update_left_border_info(current_line, cc);
	}

	store_pixels(cc, pixels ^ foreground_color);
	ccc = cc + 4;
}

void TVDecoderMono::updateScreenUpToCycle(int32 cc)
{
	uint8 pattern = sync_active ? black : background_color;
	clear_screen_up_to_cc(cc, pattern);
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

void TVDecoderMono::drawVideoBeamIndicator(int32 cc)
{
	updateScreenUpToCycle(cc);
	// TODO();
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
	cc_line_start -= cc_delta;
	cc_sync_start -= cc_delta;
	ccc -= cc_delta;
}
