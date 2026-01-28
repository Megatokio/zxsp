// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TVDecoder.h"

namespace zxsp
{

static constexpr float sec_per_scanline = 64e-6f;

template<typename T>
static inline void memset(void* z, int byte, T size)
{
	::memset(z, byte, size_t(size));
}


TVDecoder::TVDecoder(Crtc* crtc, int32 cc_per_sec, uint8 background_color) :
	cc_per_sec(cc_per_sec),
	typ_cc_per_line(int32(cc_per_sec * sec_per_scanline + 0.5f)),
	min_cc_per_line(int32(cc_per_sec * sec_per_scanline * 0.9f)),
	max_cc_per_line(int32(cc_per_sec * sec_per_scanline * 1.1f)),
	min_cc_for_vsync(int32(cc_per_sec * sec_per_scanline * 2.5f)),
	min_cc_per_frame(max_cc_per_line * min_lines_per_frame),
	max_cc_per_frame(max_cc_per_line * max_lines_per_frame),
	//fb_lines(max_cc_per_frame / min_cc_per_line + 2),
	fb_bytes_per_line(((max_cc_per_line + 15) >> 2) & ~3),
	fb_cc_per_line(fb_bytes_per_line << 2),
	crtc(crtc)
{
	cc_pixel_offset = 0;
	cc_per_line		= typ_cc_per_line;
	cc_per_frame	= cc_per_sec / 50;

	sync_active	  = false;
	cc_sync_start = 0;

	auto_position_countdown = 0;
}

TVDecoder::~TVDecoder() //
{
	delete bucket;
}

void TVDecoder::reset(int32 cc)
{
	assert(crtc->screen);
	if (!bucket) bucket = crtc->getZx80VideoData(VideoData::Zx80Frame);
	assert_ge(bucket->pixels_size, 2 * max_lines_per_frame * (max_cc_per_line + 3) / 4);

	border_attr	   = white_paper;
	current_line   = 0;
	idx_line_start = 0;
	// cc_sync_start = 0;
	cc_frame_start = cc;
	cc_line_start  = cc;
	ccc			   = cc;

	reset_auto_position_data();
}

void TVDecoder::reset_auto_position_data()
{
	first_screen_line = 0;
	last_screen_line  = 0;
	memset(cc_right, 0, sizeof(cc_right));
	memset(cc_left, 255, sizeof(cc_left));
}

inline void TVDecoder::store_pixels(int32 cc, uint8 pixels, uint8 attr)
{
	// store 8 pixels

	cc -= cc_line_start;
	int idx = cc >> 2;

	assert(idx >= 0 && idx < fb_bytes_per_line);

	uint8* p = bucket->pixel_octets + 2 * (idx_line_start + idx);

	if ((cc & 3) == 0) // byte aligned :-)
	{
		p[0] = pixels;
		p[1] = attr;
	}
	else // store across 2 bytes
	{	 // TODO: if color attributes are different then we'll get some wrong colored pixels here!
		int	  sr   = (cc & 3) * 2;
		int	  sl   = 8 - sr;
		uint8 mask = uint8(0xff << sl);

		p[0] = (p[0] & mask) | (pixels >> sr);
		p[1] = attr;
		p[2] = uint8(pixels << sl); // no need to preserve 'future' pixels
		p[3] = attr;
	}
}

void TVDecoder::clear_pixels(int32 cca, int32 cce, uint8 attr)
{
	// clear pixels with color

	cca -= cc_line_start;
	cce -= cc_line_start;

	assert(cca >= 0 && cca <= cce && cce <= fb_cc_per_line);

	uint8* p = bucket->pixel_octets + 2 * (idx_line_start + (cca >> 2));
	uint8* e = bucket->pixel_octets + 2 * (idx_line_start + ((cce + 3) >> 2));

	if (cca & 3) // border starts at odd position:
	{			 // TODO: if color attributes are different then we'll get some wrong colored pixels here!
				 // for b&w this won't happen, but for Chroma81 it may be!
		int sr	 = (cca & 3) * 2;
		int mask = 0xff >> sr;
		p[0]	 = (p[0] & ~mask); // | (attr & mask);
		//p[1]=attr;
		p += 2;
	}

	while (p < e) // TODO: use uint16ptr
	{
		*p++ = 0;	 // pixels
		*p++ = attr; // attr
	}
}

inline void TVDecoder::next_line(int32 cc)
{
	//assert_ge(cc, ccc);
	assert_le(cc, cc_line_start + max_cc_per_line);

	//	if (cc < ccc)
	//	{
	//		logline("cc < ccc: %i < %i", cc, ccc);
	//		ccc = cc;
	//	}

	//	if (cc > cc_line_start + max_cc_per_line)
	//	{
	//		logline("cc > cc_line_start + max_cc_per_line: %i > %i", cc, cc_line_start + max_cc_per_line);
	//		next_line(cc_line_start + max_cc_per_line);
	//	}

	if (cc >= ccc)
	{
		cc_line_start = ccc = cc;
		idx_line_start += fb_bytes_per_line;
		current_line += 1;

		if (current_line >= max_lines_per_frame) send_frame(cc);
	}
}

void TVDecoder::clear_screen_up_to_cc(int32 cc, uint8 attr)
{
	// clear screen up to cc with color
	// increments current_line if cc >= max_cc_per_line
	// sends a frame if current_line >= max_lines_per_frame

	if (cc > ccc)
	{
		while (cc >= cc_line_start + max_cc_per_line)
		{
			clear_pixels(ccc, cc_line_start + fb_cc_per_line, attr);
			next_line(cc_line_start + max_cc_per_line);
		}

		clear_pixels(ccc, cc, attr);
		ccc = cc;
	}
}

void TVDecoder::auto_position_screen()
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

void TVDecoder::send_frame(int32 cc)
{
	auto_position_screen();
	int h = 192;
	if unlikely (screen_position.y + h > current_line) // no sync -> may be less
	{
		logline("TVDecoder::send_frame(): screen height < 192");
		h = current_line - screen_position.y;
		assert(h > 0);
		//		if (h <= 0)
		//		{
		//			logline("TVDecoder::send_frame(): current_line <= screen_top");
		//			h = 1;
		//		}
	}

	bucket->frame  = Size {fb_bytes_per_line << 3, current_line};
	bucket->screen = {screen_position, Size {256, h}};
	bucket->cc_row = 0; //TODO
	bucket->cc_col = 0; //TODO

	crtc->sendVideoData(bucket);
	bucket = crtc->getZx80VideoData(VideoData::Zx80Frame);
	assert(bucket->pixels_size >= 2 * max_lines_per_frame * (max_cc_per_line + 3) / 4);

	current_line   = 0;
	idx_line_start = 0;
	// cc_sync_start = 0;
	cc_frame_start = cc;
	cc_line_start  = cc;
	ccc			   = cc;

	reset_auto_position_data();
}

void TVDecoder::update_right_border_info(int line, int32 cc)
{
	// called at hsync on

	cc -= cc_line_start;
	assert(cc == uint8(cc));
	cc_right[line] = uint8(cc);
}

void TVDecoder::update_left_border_info(int line, int32 cc)
{
	// called at store_pixel_byte

	if (unlikely(first_screen_line == 0)) first_screen_line = line;
	last_screen_line = line;

	cc -= cc_line_start;
	assert(cc == uint8(cc));
	cc_left[line] = uint8(cc);
}

void TVDecoder::syncOn(int32 cc, bool new_state)
{
	if (unlikely(new_state == sync_active)) return;

	if (new_state == on)
	{
		update_right_border_info(current_line, ccc);
		clear_screen_up_to_cc(cc, border_attr);
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

			//memset( bucket->pixel_octets + idx_line_start,
			//		  black,
			//		  max_lines_per_frame * fb_bytes_per_line - idx_line_start);

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

void TVDecoder::storePixelByte(int32 cc, uint8 pixels, uint8 attr)
{
	if (unlikely(sync_active)) return;

	cc += cc_pixel_offset;

	if (unlikely(cc != ccc || cc >= cc_line_start + max_cc_per_line))
	{
		clear_screen_up_to_cc(cc, border_attr);
		update_left_border_info(current_line, cc);
	}

	store_pixels(cc, pixels, attr);
	ccc = cc + 4;
}

void TVDecoder::updateScreenUpToCycle(int32 cc)
{
	uint8 attr = sync_active ? black : border_attr;
	clear_screen_up_to_cc(cc, attr);
}

void TVDecoder::setBorderColor(int32 cc, uint8 b)
{
	updateScreenUpToCycle(cc);
	border_attr = (b & 15) << 4;
}


int32 TVDecoder::getCcForFrameEnd() const
{
	// return the estimated time for the end of the current/next frame.
	// the machine will run up to this cc and probably overshoot by some cc.

	int32 cc = cc_frame_start + cc_per_frame;
	if (cc < min_cc_per_frame) return min_cc_per_frame;
	if (cc > max_cc_per_frame) return max_cc_per_frame;
	return cc;
}

void TVDecoder::drawVideoBeamIndicator(int32 cc)
{
	updateScreenUpToCycle(cc);
	// TODO();
}

int32 TVDecoder::doFrameFlyback(int32 cc)
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

void TVDecoder::shiftCcTimeBase(int32 cc_delta)
{
	cc_frame_start -= cc_delta;
	cc_line_start -= cc_delta;
	cc_sync_start -= cc_delta;
	ccc -= cc_delta;
}

} // namespace zxsp


/*



























*/
