// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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

class RenderThread : public QThread
{
	Screen* screen;
	void	run() override { screen->do_render_thread(); } // started by QThread::start()
public:
	explicit RenderThread(Screen* screen) : QThread(screen), screen(screen) {}
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

Screen::Screen(QWidget* owner, const Size& fb) :
	QGLWidget(QGLFormat(QGL::SingleBuffer), owner),
	current_frame(fb.width, fb.height),
	thread(new RenderThread(this)),
	zoom(calc_zoom())
{
	xlogIn("new Screen");

	setAttribute(Qt::WA_OpaquePaintEvent, 1); // we paint all pixels
	// setAttribute(Qt::WA_NoSystemBackground,1);	// the widget has transparent parts
	setAutoFillBackground(false); // else ctors of Painters in paint_screen() will erase whole Screen

	assert(context()->isValid());
	doneCurrent(); // release OGL context so that render_thread can aquire it

#if QT_VERSION >= 0x050000
	context()->moveToThread(thread);
	xlogline("moved context to render_thread");
#endif

	thread->start(); // start default run() which calls exec() to run the event loop
}

Screen::~Screen()
{
	xlogIn("~Screen");

	termi = true;
	in_queue.sema.release();
	thread->wait();
	delete thread;
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
	repaint(); // QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::resizeEvent(QResizeEvent*)
{
	assert(isMainThread());
	calc_zoom();
	repaint(); // QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::repaint()
{
	// store a repaint request.
	// to be used in paintGL(), resizeGL(w,h), paintEvent() and resizeEvent().
	// the method waits until the render thread has painted the entire screen.

	_repaint = true;
	wait_repaint_sema.acquire(); // TODO: geht das auch ohne?
}

void Screen::saveScreenshot(cstr path)
{
	assert(isMainThread());

	// replace screen.out_queue with own queue:
	FrameDataQueue* qout = out_queue; // remember
	FrameDataQueue	qin;			  // new queue
	out_queue = &qin;				  // replace out_queue

	// wait for next framedata:
	while (!qin.avail() && !termi) { qin.sema.acquire(); }

	// save screenshot:
	if (qin.avail())
	{
		FrameData* framedata = qin.get();
		GifWriter::saveScreenshot(path, framedata);
		qout->put(framedata);
	}

	// restore screen.out_queue:
	FrameDataQueue::mutex.lock();
	out_queue = qout;
	FrameDataQueue::mutex.unlock();
	qin.flush_to(qout);
}

void Screen::startRecording(cstr path, bool with_border)
{
	assert(isMainThread());
	if (gif_recorder) return;

	gif_recorder = new GifRecorder(out_queue);
	out_queue	 = &gif_recorder->in_queue;
	gif_recorder->startRecording(path, with_border, 50);
}

void Screen::stopRecording()
{
	assert(isMainThread());
	if (!gif_recorder) return;

	gif_recorder->stopRecording();

	// restore screen.out_queue:
	// TODO: if multiple long running patches are possible then we may restore the wrong ptr!
	FrameDataQueue::mutex.lock();
	out_queue = gif_recorder->out_queue;
	FrameDataQueue::mutex.unlock();
	gif_recorder->in_queue.flush_to(out_queue);

	delete gif_recorder;
	gif_recorder = nullptr;
}

void Screen::do_render_thread()
{
	// run the event loop of the render thread.
	// waits for requests and executes them.

	try
	{
		for (;;)
		{
			in_queue.sema.acquire();
			if (termi) break;

			if (_repaint)
			{
				_repaint = false;
				if (isVisible()) do_draw_screen(true);
				wait_repaint_sema.release();
			}

			if (in_queue.avail())
			{
				FrameData* framedata = in_queue.get();
				if (!in_queue.avail() && isVisible())
				{
					zxspRenderer(&current_frame, framedata);
					do_draw_screen(false);
				}
				assert(out_queue);
				put(out_queue, framedata); // also locks the pointer
			}
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

void Screen::draw_rect(int x, int y, int w, int h, RgbaColor color)
{
	glRasterPos2i(x, y); // window coordinates
	glPixelZoom(w, -h);
	glDrawPixels(1, 1, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, &color);
}

void Screen::do_draw_screen(bool draw_passepartout)
{
	makeCurrent();

	// create painter (for drawing overlays) but first do native openGL painting:
	QPainter p(this);
	p.beginNativePainting();
	(void)glGetError(); // clear error

	// update viewport ((imageable area))
	// glViewport specifies the affine transformation of x and y
	// from normalized device coordinates to window coordinates.
	glViewport(0, 0, width(), height());

	// setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// glOrtho describes a transformation that produces a parallel projection.
	// The current matrix is multiplied by this matrix
	// and the result replaces the current matrix
	glOrtho(0, width(), height(), 0, -1, 1); // left, right, bottom, top, near_, far_clipping_plane

	// setup new_pixels unpacking, transfer, mapping & rasterization
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// setup geometry
	// all coordinates are measured in standard zxsp pixels, e.g. screen = 256x192:
	const VideoFrame& cf = current_frame;

	int zoom = minmax(1, min(width() / 256, height() / 192), 4); // getZoom()
	int w	 = (width() + zoom - 1) / zoom;						 // widget size
	int h	 = (height() + zoom - 1) / zoom;					 //
	int x0	 = (w - cf.frameWidth() / cf.hf + 1) / 2;			 // position of frame inside widget
	int y0	 = (h - cf.frameHeight() + 1) / 2;					 // mostly negative!

	// draw passpartout, if required:
	//	note on cpu usage:							zoom=2	fullscreen		(oGL only, no Painter)
	//	passepartout only drawn when requested		8.1%	7.5%			approx.
	//	passepartout always drawn, border only		10.6%	16.6%			approx.
	//	passepartout always drawn, full rect		18.5%	20.2%			approx.
	//
	if (draw_passepartout)
	{
		// measurement in real (widget) pixels:
		int frame_height = cf.frameHeight() * zoom;
		int left_black	 = x0 * zoom;
		int top_black	 = y0 * zoom;
		int bottom_black = height() - (top_black + frame_height);
		int right_black	 = width() - (left_black + cf.frameWidth() / cf.hf * zoom);

		if (top_black > 0) draw_rect(0, 0, width(), top_black, black);
		if (left_black > 0) draw_rect(0, top_black, left_black, frame_height, black);
		if (bottom_black > 0) draw_rect(0, height() - bottom_black, width(), bottom_black, black);
		if (right_black > 0) draw_rect(width() - right_black, top_black, right_black, frame_height, black);
	}

	// setup new pixels unpacking, transfer, mapping & rasterization:
	glRasterPos2i(x0 * zoom, y0 * zoom); // window coordinates
	glPixelZoom(GLfloat(zoom) / cf.hf, -zoom);
	glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(RgbaColor)); // if RGBA
	glPixelStorei(GL_UNPACK_ROW_LENGTH, cf.frameWidth());  // number of pixels
	// note: glDrawPixels(w,h,format,type,data*)
	glDrawPixels(cf.frameWidth(), cf.frameHeight(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, cf.pixels);

	uint err = glGetError();
	if (err) logline("OpenGL error: $%04x", err);

	// flush drawing to screen:
	// without Painter, resize() was (nearly) flicker-free:
	//
	//	if(doubleBuffer()) swapBuffers();
	//	else glFlush();				// flush all buffered commands to the GPU
	//	//else glFinish();			// also blocks until done

	// doneCurrent();

	p.endNativePainting();
	draw_overlays(p, zoom);
	if (loglevel >= 1) log_sysload();
}

void Screen::draw_overlays(QPainter& p, int zoom)
{
	p.setBackgroundMode(Qt::BGMode::TransparentMode);
	p.scale(zoom, zoom);

	if (auto* ov = rzx_overlay.get())
	{
		int w = (width() + zoom - 1) / zoom; // window size
		int h = (height() + zoom - 1) / zoom;

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
}

void Screen::setRzxOverlay(const RzxOverlayPtr& p)
{
	if (rzx_overlay == p) return;

	mutex.lock();
	rzx_overlay = p;
	mutex.unlock();
}

void Screen::setJoystickOverlay(uint index, const JoystickOverlayPtr& p)
{
	assert(index < NELEM(joystick_overlays));

	if (joystick_overlays[index] == p) return;

	mutex.lock();
	joystick_overlays[index] = p;
	mutex.unlock();
}

void Screen::setNumJoystickOverlays(uint n)
{
	mutex.lock();
	while (n < NELEM(joystick_overlays))
	{
		joystick_overlays[n++] = nullptr; //
	}
	mutex.unlock();
}

void Screen::removeAllOverlays()
{
	mutex.lock();
	rzx_overlay = nullptr;
	for (uint i = 0; i < NELEM(joystick_overlays); i++)
	{
		joystick_overlays[i] = nullptr; //
	}
	mutex.unlock();
}

} // namespace zxsp
