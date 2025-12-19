// Copyright (c) 2025 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "IoInfo.h"
#include "Templates/Queue.h"
#include "graphics/geometry.h"
#include <QMutex>
#include <QSemaphore>
#include <QThread>


// this file defines:
//
// class VideoData and subclasses for Zxsp++ and Zx80++
// class IVideoDataReceiver, the interface for video data receivers
// class VideoDataReceiver for regular receivers with worker thread
// class VideoDataPool, a cache for recycling of VideoData containers
// class IScreen, the interface for sending VideoData to the screen
//		 and retrieving new containers from a VideoDataPool


namespace zxsp
{

using coord = int32;
using Point = geometry::Point<coord>;
using Size	= geometry::Size<coord>;
using Dist	= geometry::Dist<coord>;
using Rect	= geometry::Rect<coord>;

/*
   class VideoData provides a container for video data sent to the Screen and VideoRecorder.
   It comes currently in two subtypes for ZXSP fixed frames and ZX80 soft frames,
   each with a regular and an auxiliary variant for incomplete frames with videobeam position.
   New VideoData is normally obtained from a VideoDataPool which is created by the Screen,
   and sent back to the Screen for display or delete'ed.

   A VideoData's life:
   The Crtc get's a VideoData container from the Pool, fills it and send's it
   to the Screen which directly or indirectly returns it back to the Pool.
   Pool --> Crtc --> Screen --> VideoRecorder --> Pool
*/

struct VideoData
{
	enum What : uint8 { Zx80Frame, ZxspFrame, Tc2048Frame, SpectraFrame };
	What what;
	bool aux;
	bool flashphase = 0;

	VideoData(What w, bool aux) noexcept : what(w), aux(aux) {}
	virtual ~VideoData() = default;
};

struct ZxspVideoData : public VideoData
{
	uint8*	attrpixels			   = nullptr;
	IoInfo* ioinfo				   = nullptr;
	int		pixels_size			   = 0;
	int		ioinfo_size			   = 0;
	int		ioinfo_count		   = 0;
	int		cc_per_scanline		   = 0;
	int32	cc_start_of_screenfile = 0;
	int32	cc					   = 0;

	ZxspVideoData(What what, bool aux, int size);
	ZxspVideoData(bool aux) : ZxspVideoData(ZxspFrame, aux, 32 * 24 * 8 * 2) {}
	~ZxspVideoData() override;
	void record_io(int32 cc, uint16 addr, uint8 byte);
};

struct SpectraVideoData : public ZxspVideoData
{
	SpectraVideoData(bool aux) : ZxspVideoData(SpectraFrame, aux, 32 * 24 * 8 * 3) {}
};

struct Tc2048VideoData : public ZxspVideoData
{
	Tc2048VideoData(bool aux) : ZxspVideoData(Tc2048Frame, aux, 32 * 24 * 8 * 2) {}
};

struct Zx80VideoData : public VideoData
{
	uint8* pixel_octets = nullptr;
	int32  pixels_size	= 0;
	Size   frame {256 + 2 * 64, 192 + 2 * 48};
	Rect   screen {64, 48, 256, 192};
	int	   cc_row = 0;
	int	   cc_col = 0;

	Zx80VideoData(bool aux);
	~Zx80VideoData() override;
};


// =========================================================================

/*
   class IVideoDataReceiver provides a sender-receiver interface for VideoData.
   sending and receiving is not thread-safe: it is assumed that there is only one sender
   or that sending is externally synchronized if needed.
   linking/unlinking is thread-safe, but all access to _next and _prev in subclasses
	must also be protected by locking _mutex, e.g. when forwarding VideoData!
*/

class IVideoDataReceiver
{
public:
	IVideoDataReceiver() = default;
	virtual ~IVideoDataReceiver() noexcept;

	void link(IVideoDataReceiver*); // behind this
	void unlink();

	virtual void stop() {}								 // if rcvr uses a thread
	virtual void sendVideoData(VideoData*) noexcept = 0; // for peer

protected:
	static QMutex		_mutex; // --> _next, _prev
	IVideoDataReceiver* _next = nullptr;
	IVideoDataReceiver* _prev = nullptr;
};


// =========================================================================

/*
   class VideoDataReceiver provides a buffered sender-receiver interface for VideoData.
   Sending data releases a semaphore and receiving data retains it,
   so the receiver will block while there is no data available in the queue.
   If the receiver thread is started with start() then it can be stopped & deleted with stop().
*/

class VideoDataReceiver : public IVideoDataReceiver
{
public:
	VideoDataReceiver() = default;
	~VideoDataReceiver() noexcept override;

	int	 avail() const noexcept { return _queue.avail(); }
	int	 free() const noexcept { return _queue.free(); }
	void start(QThread*);
	void stop() override;							  // wait to join & delete thread
	void sendVideoData(VideoData*) noexcept override; // for peer --> queue

protected:
	VideoData* getVideoData() noexcept;			  // get data from queue
	void	   fwdVideoData(VideoData*) noexcept; // forward data to _next

	kio::Queue<VideoData*, 4> _queue;
	QSemaphore				  _sema;
	QThread*				  _thread = nullptr;
	bool					  _termi  = false;
};


// =========================================================================

/*
   class VideoDataPool is the source and the final receiver of all VideoData containers.
   class VideoDataPool is normally incorporated into the Screen by using interface IScreen.

   The pool creates new VideoData and destroys them if required.
   Both sending and retrieving VideoData is NOT thread-safe.
   It is assumed that only one instance (the Crtc) retrieves VideoData containers,
   and only one instance (the Screen) is returning them.
   Otherwise any access must be properly synchronized.
*/

class VideoDataPool : public IVideoDataReceiver
{
public:
	VideoData* getVideoData(VideoData::What, bool aux = no);
	void	   sendVideoData(VideoData*) noexcept override; // return to pool
	~VideoDataPool() noexcept override;

private:
	kio::Queue<VideoData*, 2> pools[2]; // [0]=regular, [1]=aux
};


// =========================================================================

/*
   class IScreen provides an interface for buffered sending VideoData to the screen
   and retrieving empty VideoData containers from a per-screen cache pool.

   Both sending and retrieving VideoData is NOT thread-safe.
   It is assumed that only one instance (the Crtc) is talking to the Screen,
   any other access must be synchronized, e.g. by suspending the Machine.
*/

class IScreen : public VideoDataReceiver
{
public:
	using VideoDataReceiver::sendVideoData;					 // to the screen
	VideoData* getVideoData(VideoData::What, bool aux = no); // from pool

protected:
	VideoDataPool _pool;
};


//
// ----------------------------------
//		Inline Implementations
// ----------------------------------
//

inline void ZxspVideoData::record_io(int32 cc, uint16 addr, uint8 byte)
{
	// provided for ZxspUla:
	// record out() instructions for border color change

	if unlikely (ioinfo_count == ioinfo_size)
	{
		// reallocating ioinfo[] is a little bit dangerous if aux frames were sent!
		// but this will only happen in extreme cases.
		// tape save routine: ~80  out/frame
		// speed loader 5kHz: ~200 out/frame
		// chequered border:  4368 out/frame (32x32 pixels)  == 272*16
		// chequered border:  8736 out/frame (16x16 pixels)  == 272*32
		assert(ioinfo_size >= 200);
		ioinfo_size *= 2;
		IoInfo* z = new IoInfo[ioinfo_size + 1]; // 1 more for renderer
		memcpy(z, ioinfo, ioinfo_count * sizeof(IoInfo));
		delete[] ioinfo;
		ioinfo = z;
	}
	ioinfo[ioinfo_count++] = IoInfo(cc, addr, byte);
}

inline VideoData* IScreen::getVideoData(VideoData::What what, bool aux) //
{
	return _pool.getVideoData(what, aux);
}


} // namespace zxsp

/*




























*/
