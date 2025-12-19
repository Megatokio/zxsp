// Copyright (c) 2025 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Libraries/kio/kio.h"
#include "Templates/Queue.h"
#include "graphics/geometry.h"
#include <QMutex>
#include <QSemaphore>
#include <QThread>
struct IoInfo;


namespace zxsp
{

using coord = int32;
using Point = geometry::Point<coord>;
using Size	= geometry::Size<coord>;
using Dist	= geometry::Dist<coord>;
using Rect	= geometry::Rect<coord>;


struct FrameData
{
	enum { Zx80Frame, ZxspFrame, Tc2048Frame, SpectraFrame } what;
};

struct ZxspFrameData : public FrameData
{
	uint8*	attrpixels			   = nullptr;
	IoInfo* ioinfo				   = nullptr;
	int		ioinfo_count		   = 0;
	int		cc_per_scanline		   = 0;
	int32	cc_start_of_screenfile = 0;
	int32	cc					   = 0;
	bool	flashphase			   = 0;
};

struct Zx80FrameData : public FrameData
{
	uint8* pixel_octets = nullptr;
	Size   frame {256 + 2 * 64, 192 + 2 * 48};
	Rect   screen {64, 48, 256, 192};
	int32  cc = 0;
};


class FrameDataQueue : public kio::Queue<FrameData*, 4>
{
public:
	static QMutex mutex;
	QSemaphore	  sema;

	using Queue::avail;
	using Queue::free;
	using Queue::get;

	void put(FrameData* c) noexcept
	{
		mutex.lock();
		Queue::put(std::move(c));
		sema.release();
		mutex.unlock();
	}

	void put_unlocked(FrameData* c) noexcept
	{
		Queue::put(std::move(c));
		sema.release();
	}

	void flush_to(FrameDataQueue* z)
	{
		while (avail()) z->put(get());
	}
};

inline void put(FrameDataQueue* q, FrameData* d)
{
	// put FrameData into queue q
	// also lock access to the pointer q itself,
	// if it may be changed concurrently any time

	FrameDataQueue::mutex.lock();
	q->put_unlocked(std::move(d));
	FrameDataQueue::mutex.unlock();
}

} // namespace zxsp
