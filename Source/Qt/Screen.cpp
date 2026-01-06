// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define loglevel 0

#include "Screen.h"
#include "IsaObject.h"
#include "Machine.h"
#include "MachineController.h"
#include "Overlays/Overlay.h"
#include "Renderer.h"
#include "Settings.h"
#include "unix/os_utilities.h"
#include <QApplication>
#include <QGLFormat>
#include <QGLWidget>
#include <QMutex>
#include <QSemaphore>
#include <QThread>


namespace zxsp
{

static void log_sysload()
{
	static int n;
	if ((++n & 0xFF) == 0)
	{
		double loads[3];
		sysLoad(loads);

		static double average = 0.1;
		double		  load	  = cpuLoad();
		average				  = average * 0.9 + load * 0.1;
		logline("sysLoad = %1.3f, cpuLoad = %.2f%% (avg = %.2f%%)", loads[0], load * 100, average * 100);
	}
}

// =========================================================================
//							Render Thread
// =========================================================================

class ScreenUpdateThread : public QThread
{
	Screen* screen;
	void	run() override { screen->do_render_thread(); } // started by QThread::start()
public:
	explicit ScreenUpdateThread(Screen* screen) : QThread(screen), screen(screen) //
	{
		setObjectName("ScreenUpdateThread");
	}
};


// =========================================================================
//							c'tor, d'tor
// =========================================================================

/*
Constant					Description
QGL::DoubleBuffer			Specifies the use of double buffering.
QGL::DepthBuffer			Enables the use of a depth buffer.
QGL::Rgba					Specifies that the context should use RGBA as its pixel format.
QGL::AlphaChannel			Enables the use of an alpha channel.
QGL::AccumBuffer			Enables the use of an accumulation buffer.
QGL::StencilBuffer			Enables the use of a stencil buffer.
QGL::StereoBuffers			Enables the use of a stereo buffers for use with visualization hardware.
QGL::DirectRendering		Specifies that the context is used for direct rendering to a display.
QGL::HasOverlay				Enables the use of an overlay.
QGL::SampleBuffers			Enables the use of sample buffers.
QGL::DeprecatedFunctions	Enables the use of deprecated functionality for OpenGL 3.x contexts. A context with
							deprecated functionality enabled is called a full context in the OpenGL specification.
QGL::SingleBuffer			Specifies the use of a single buffer, as opposed to double buffers.
QGL::NoDepthBuffer			Disables the use of a depth buffer.
QGL::ColorIndex				Specifies that the context should use a color index as its pixel format.
QGL::NoAlphaChannel			Disables the use of an alpha channel.
QGL::NoAccumBuffer			Disables the use of an accumulation buffer.
QGL::NoStencilBuffer		Disables the use of a stencil buffer.
QGL::NoStereoBuffers		Disables the use of stereo buffers.
QGL::IndirectRendering		Specifies that the context is used for indirect rendering to a buffer.
QGL::NoOverlay				Disables the use of an overlay.
QGL::NoSampleBuffers		Disables the use of sample buffers.
QGL::NoDeprecatedFunctions	Disables the use of deprecated functionality for OpenGL 3.x contexts (forward compatible)
*/

Screen::Screen(QWidget* owner) : //
	QGLWidget(QGLFormat(QGL::SingleBuffer), owner)
{
	xlogIn("new Screen");

	link(&_pool);
	calc_zoom();

	setAttribute(Qt::WA_OpaquePaintEvent, 1); // we paint all pixels
	// setAttribute(Qt::WA_NoSystemBackground,1);	// the widget has transparent parts
	setAutoFillBackground(false); // else ctors of Painters in paint_screen() will erase whole Screen

	assert(context()->isValid());
	doneCurrent(); // release OGL context so that render_thread can aquire it

	QThread* worker = new ScreenUpdateThread(this);

#if QT_VERSION >= 0x050000
	context()->moveToThread(worker);
	xlogline("moved context to render_thread");
#endif

	start(worker); // start default run() which runs the event loop
}

Screen::~Screen()
{
	xlogIn("~Screen");

	stop(); // own thread

	while (_next)
	{
		_next->stop();
		_next->unlink();
	}

	delete gif_recorder;
}

void Screen::initializeGL()
{
	// Setup resources needed by the OpenGL implementation to render the scene.
	QGLWidget::initializeGL();
}


// =========================================================================
//							Methods
// =========================================================================

void Screen::paintGL() { abort("Screen::paintGL() called!"); }

void Screen::resizeGL(int, int) { abort("Screen::resizeGL(int,int) called!"); }

void Screen::paintEvent(QPaintEvent*)
{
	assert(isMainThread());
	//repaint();  we get 50 new frames per second anyway
	// QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::resizeEvent(QResizeEvent*)
{
	assert(isMainThread());
	calc_zoom();
	//repaint();  we get 50 new frames per second anyway
	// QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

//void Screen::repaint()
//{
//	// store a repaint request.
//	// to be used in paintGL(), resizeGL(w,h), paintEvent() and resizeEvent().

//	no longer used

//	_repaint = true;
//	_sema.release();
//}

void Screen::saveScreenshot(cstr path) //
{
	link(new GifWriter(path));
}

void Screen::startRecording(cstr path, bool with_border)
{
	assert(isMainThread());

	if (gif_recorder) return;
	gif_recorder = new GifRecorder(path, with_border, 50);
	link(gif_recorder);
}

void Screen::stopRecording()
{
	assert(isMainThread());

	if (!gif_recorder) return;
	gif_recorder->unlink();
	gif_recorder->stopRecording();
	delete gif_recorder;
	gif_recorder = nullptr;
}

void Screen::do_render_thread()
{
	// run the event loop of the render thread.
	// waits for requests and executes them.

	try
	{
		while (!_termi)
		{
			if (VideoData* framedata = VideoDataReceiver::getVideoData())
			{
				if (isVisible())
				{
					zxspRenderer(&current_frame, framedata);
					do_draw_screen();
				}
				fwdVideoData(framedata);
			}
			//			else if (_repaint)
			//			{
			//				if (isVisible() && current_frame.pixels) do_draw_screen();
			//			}
		}
	}
	catch (std::exception& e)
	{
		showAlert("screen render thread crashed: %s", e.what());
	}

#if QT_VERSION >= 0x050000
	context()->moveToThread(QApplication::instance()->thread());
	xlogline("moved context back to gui thread");
#endif
}

void Screen::do_draw_screen()
{
	QPainter p(this);
	//_repaint = false;

	const VideoFrame& cf = current_frame;
	assert(cf.pixels);

	// all coordinates are in zxsp screen pixels [256x192]:
	int w	 = width();
	int h	 = height();
	int zoom = minmax(1, min(w / 256, h / 192), 4);

	w = (w + zoom - 1) / zoom;
	h = (h + zoom - 1) / zoom;

	int x0 = (w - cf.frameWidth() / cf.hf) / 2; // position of frame inside widget
	int y0 = (h - cf.frameHeight()) / 2;		// mostly negative!

	p.scale(zoom, zoom);

	// draw passepartout
	if (y0 >= 0) // top + bottom
	{
		const int bottom_black = h - cf.frameHeight() - y0;
		p.fillRect(0, 0, w, y0, black);
		p.fillRect(0, h - bottom_black, w, bottom_black, black);
	}
	if (x0 >= 0) // left + right
	{
		const int right_black = w - cf.frameWidth() / cf.hf - x0;
		p.fillRect(0, y0, x0, cf.frameHeight(), black);
		p.fillRect(w - right_black, y0, right_black, cf.frameHeight(), black);
	}

	QImage img(ucharptr(cf.pixels), cf.frameWidth(), cf.frameHeight(), QImage::Format_RGB32);
	p.drawImage(QRect(x0, y0, cf.frameWidth() / cf.hf, cf.frameHeight()), img);

	// draw overlays:
	if (rzx_overlay || joystick_overlays[0])
	{
		p.setBackgroundMode(Qt::BGMode::TransparentMode);

		overlay_mutex.lock();

		if (auto* ov = rzx_overlay.get())
		{
			ov->x = w - ov->w;
			ov->y = h - ov->h;
			ov->draw(p, zoom);
		}

		if (joystick_overlays[0] && settings.get_bool(key_show_joystick_overlays, true)) // TODO cache
		{
			p.translate(2, 2);
			for (uint i = 0; i < NELEM(joystick_overlays); i++)
			{
				auto* ov = joystick_overlays[i].get();
				if (!ov) break;
				ov->x = 0;
				ov->y = 0;
				ov->draw(p, zoom);
				p.translate(0, ov->h + 2);
			}
		}

		overlay_mutex.unlock();
	}

	if (loglevel >= 1) log_sysload();
}

void Screen::setRzxOverlay(const RzxOverlayPtr& p)
{
	if (rzx_overlay == p) return;

	overlay_mutex.lock();
	rzx_overlay = p;
	overlay_mutex.unlock();
}

void Screen::setJoystickOverlay(uint index, const JoystickOverlayPtr& p)
{
	assert(index < NELEM(joystick_overlays));

	if (joystick_overlays[index] == p) return;

	overlay_mutex.lock();
	joystick_overlays[index] = p;
	overlay_mutex.unlock();
}

void Screen::setNumJoystickOverlays(uint n)
{
	overlay_mutex.lock();
	while (n < NELEM(joystick_overlays))
	{
		joystick_overlays[n++] = nullptr; //
	}
	overlay_mutex.unlock();
}

void Screen::removeAllOverlays()
{
	overlay_mutex.lock();
	rzx_overlay = nullptr;
	for (uint i = 0; i < NELEM(joystick_overlays); i++)
	{
		joystick_overlays[i] = nullptr; //
	}
	overlay_mutex.unlock();
}

} // namespace zxsp
