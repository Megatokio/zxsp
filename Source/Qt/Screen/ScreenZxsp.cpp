// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 0
#include "ScreenZxsp.h"
#include "IsaObject.h"
#include "Machine.h"
#include "MachineController.h"
#include "MonoRenderer.h"
#include "Overlays/Overlay.h"
#include "SpectraRenderer.h"
#include "Tc2048Renderer.h"
#include "ZxspRenderer.h"
#include "unix/os_utilities.h"
#include <QApplication>
#include <QGLFormat>
#include <QGLWidget>
#include <QMutex>
#include <QSemaphore>
#include <QThread>


namespace gui
{

bool ScreenZxsp::ffb_or_vbi(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint32 cc_start_of_screenfile, uint cc_per_scanline,
	bool flashphase, uint32 cc)
{
	// store data for new FFB and trigger render thread.
	// the arrays ioinfo[] and attr_pixels[] are managed by the caller.
	// they are never deleted by this Screen or RenderThread.
	// returns true  if new buffers must be retained and old buffers may now be reused.
	// returns false if new buffers may be reused and old buffers must remain retained.

	_mutex.lock();

	bool ffb_ready = ~_what & FFB_OR_VBI;
	frames_hit_percent *= 0.98f;
	if (ffb_ready)
	{
		frames_hit_percent += 2.0f;

		_what |= FFB_OR_VBI;
		_ioinfo					= ioinfo;
		_ioinfo_count			= ioinfo_count;
		_attrpixels				= attr_pixels;
		_cc_start_of_screenfile = cc_start_of_screenfile;
		_cc_per_scanline		= cc_per_scanline;
		_cc						= cc;
		_flashphase				= flashphase;
	}

	_mutex.unlock();

	if (ffb_ready) _sema.release();
	return ffb_ready; // true --> retain new data for processing in progress, else still retain old data
}

void ScreenZxsp::do_ffb_or_vbi()
{
	uint8*	attrpixels			   = _attrpixels;
	IoInfo* ioinfo				   = _ioinfo;
	uint	ioinfo_count		   = _ioinfo_count;
	uint	cc_per_scanline		   = _cc_per_scanline;
	uint32	cc_start_of_screenfile = _cc_start_of_screenfile;
	uint32	cc					   = _cc;
	bool	flashphase			   = _flashphase;
	_mutex.unlock();

	if (isVisible()) // --> draw screen
	{
		assert(dynamic_cast<ZxspRenderer*>(screen_renderer));
		static_cast<ZxspRenderer*>(screen_renderer)
			->drawScreen(ioinfo, ioinfo_count, attrpixels, cc_per_scanline, cc_start_of_screenfile, flashphase, cc);
		paint_screen(no);
	}

	if (_screenshot_filepath) // --> make a screenshot
	{
		cstr path			 = _screenshot_filepath;
		_screenshot_filepath = nullptr;

		GifWriter* gif = newGifWriter(no, 50);
		assert(dynamic_cast<ZxspGifWriter*>(gif));

		try
		{
			static_cast<ZxspGifWriter*>(gif)->saveScreenshot(
				path, ioinfo, ioinfo_count, attrpixels, cc_per_scanline, cc_start_of_screenfile);
		}
		catch (FileError& e)
		{
			showWarning("File error: %s", e.what());
		}
		delete[] path;
		delete gif;
	}

	if (_gifmovie_filepath) // --> start recording into an animated gif
	{
		cstr path		   = _gifmovie_filepath;
		bool with_border   = _gifmovie_with_bordereffects;
		_gifmovie_filepath = nullptr;

		gif_writer = newGifWriter(with_border, 50);
		assert(dynamic_cast<ZxspGifWriter*>(gif_writer));

		try
		{
			gif_writer->startRecording(path);
		}
		catch (FileError& e)
		{
			showWarning("File error: %s", e.what());
			delete gif_writer;
			gif_writer = nullptr;
		}
		delete[] path;
	}

	if (gif_writer) // --> record frame into gif movie file
	{
		try
		{
			assert(dynamic_cast<ZxspGifWriter*>(gif_writer));
			static_cast<ZxspGifWriter*>(gif_writer)
				->writeFrame(ioinfo, ioinfo_count, attrpixels, cc_per_scanline, cc_start_of_screenfile, flashphase);
		}
		catch (FileError& e)
		{
			showWarning("File error: %s", e.what());
			delete gif_writer;
			gif_writer = nullptr;
		}
	}
}


} // namespace gui
