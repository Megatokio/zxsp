// Copyright (c) 2023 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "graphics/geometry.h"
#include "isa_id.h"

namespace zxsp
{
struct IoInfo;

using coord = int32;
using Point = geometry::Point<coord>;
using Size	= geometry::Size<coord>;
using Dist	= geometry::Dist<coord>;
using Rect	= geometry::Rect<coord>;


/*
	Interface for real screens (widget or window)
	both color and b&w are handled in one interface to avoid the diamond problem.
	the different send-frame functions should be consolidated anyway.
*/

class IScreen
{
public:
	IScreen() noexcept			= default;
	virtual ~IScreen() noexcept = default;

	// function to send a frame
	// all metrics in pixels.
	// return true  if new buffers must be retained and old buffers may now be reused
	// return false if new buffers may be reused and old buffers must remain retained

	// Monochrome:
	virtual bool sendFrame(uint8* frame_data, const zxsp::Size& frame_size, const zxsp::Rect& screen) = 0;

	// Color:
	virtual bool ffb_or_vbi(
		IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint32 cc_start_of_screenfile, uint cc_per_scanline,
		bool flashphase, uint32 cc) = 0;
};


class NoScreen : public IScreen
{
public:
	bool sendFrame(uint8*, const zxsp::Size&, const zxsp::Rect&) override { return true; }
	bool ffb_or_vbi(IoInfo*, uint, uint8*, uint32, uint, bool, uint32) override { return true; }
};

} // namespace zxsp

/*
















*/
