// Copyright (c) 2025 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "VideoData.h"

namespace zxsp
{
QMutex IVideoDataReceiver::_mutex;

Zx80VideoData::Zx80VideoData(bool aux) : VideoData(Zx80Frame, aux)
{
	if (aux) return; // aux has no own array
	pixels_size	 = (312 + 32) * (208 + 32) / 4;
	pixel_octets = new uint8[pixels_size];
}

Zx80VideoData::~Zx80VideoData()
{
	if (aux) return;
	delete[] pixel_octets;
}

ZxspVideoData::ZxspVideoData(What what, bool aux, int size) : VideoData(what, aux)
{
	if (aux) return; // aux has no own arrays
	ioinfo	   = new IoInfo[ioinfo_size = 272];
	attrpixels = new uint8[pixels_size = size];
}

ZxspVideoData::~ZxspVideoData()
{
	if (aux) return;
	delete[] attrpixels;
	delete[] ioinfo;
}


// ############################################################################

IVideoDataReceiver::~IVideoDataReceiver() noexcept
{
	unlink(); // safety only
}

void IVideoDataReceiver::link(IVideoDataReceiver* v)
{
	_mutex.lock();
	assert(v->_next == nullptr && v->_prev == nullptr);
	v->_next = _next;
	v->_prev = this;
	_next	 = v;
	_mutex.unlock();
}

void IVideoDataReceiver::unlink()
{
	_mutex.lock();
	if (_next) _next->_prev = _prev;
	if (_prev) _prev->_next = _next;
	_next = _prev = nullptr;
	_mutex.unlock();
}


// ############################################################################

VideoDataReceiver::~VideoDataReceiver() noexcept
{
	stop(); // safety only
	while (avail()) { delete _queue.get(); }
}

void VideoDataReceiver::sendVideoData(VideoData* v) noexcept
{
	// store VideoData into the receiver queue:
	// if queue not empty: discard aux VideoData
	// if queue is full:   wait (should never happen)

	if (v->aux)
	{
		if (_queue.avail())
		{
			delete v;
			return;
		}
	}
	else // regular frame
	{
		while (_queue.free() == 0) { usleep(500); }
	}

	_queue.put(v);
	_sema.release();
}

void VideoDataReceiver::start(QThread* t)
{
	_thread = t;
	_thread->start();
}

void VideoDataReceiver::stop()
{
	// send message to _thread that it should terminate and delete _thread:
	// the receiver will receive a nullptr in pullVideoData().

	if (_thread)
	{
		_termi = true;
		_sema.release();
		_thread->wait();
		delete _thread;
		_thread = nullptr;
	}
}

VideoData* VideoDataReceiver::getVideoData() noexcept
{
	// wait for and return next VideoData:
	// nullptr: terminate

	_sema.acquire();
	return _queue.avail() ? _queue.get() : nullptr;
}

void VideoDataReceiver::fwdVideoData(VideoData* data) noexcept
{
	// send VideoData to the next consumer:
	// this is most likely the VideoDataPool.
	// lock the mutex because the gui can add/remove receivers at any time!

	_mutex.lock();
	assert(_next);
	_next->sendVideoData(data);
	_mutex.unlock();
}


// ############################################################################


VideoDataPool::~VideoDataPool() noexcept
{
	for (uint i = 0; i < NELEM(pools); i++)
	{
		while (pools[i].avail()) delete pools[i].get();
	}
}

void VideoDataPool::sendVideoData(VideoData* bucket) noexcept
{
	// return VideoData container to the pool:
	// if no space available then simply dispose it.

	if (!bucket) return;

	auto& pool = pools[bucket->aux];

	if (pool.free()) pool.put(bucket);
	else delete bucket;
}

VideoData* VideoDataPool::getVideoData(VideoData::What what, bool aux)
{
	// get a VideoData with a certain type:
	// normally only one type is in circulation.
	// if no VideoData of the requested type is available, create one.

	auto& pool = pools[aux];

	if (pool.avail()) // 99%
	{
		VideoData* bucket = pool.get();
		if (bucket->what == what) return bucket; // 99.999%
		else delete bucket;
	}

	switch (what)
	{
	case VideoData::Zx80Frame: return new Zx80VideoData(aux);
	case VideoData::ZxspFrame: return new ZxspVideoData(aux);
	case VideoData::Tc2048Frame: return new Tc2048VideoData(aux);
	case VideoData::SpectraFrame: return new SpectraVideoData(aux);
	default: TODO();
	}
}


} // namespace zxsp

/*
























*/
