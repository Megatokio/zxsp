// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "graphics/geometry.h"


using coord = int32;
using Point = geometry::Point<coord>;
using Size = geometry::Size<coord>;
using Dist = geometry::Dist<coord>;
using Rect = geometry::Rect<coord>;


class IScreenMono
{
protected:
	virtual ~IScreenMono()=default;

public:
	// function to call to emit a frame
	// all metrics in pixels.
	// return true  if new buffers must be retained and old buffers may now be reused
	// return false if new buffers may be reused and old buffers must remain retained
	virtual bool sendFrame(uint8* frame_data, const Size& frame_size, const Rect& screen) = 0;
};


