// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 0
#include "Screen.h"
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

Screen::Screen(QWidget* owner, isa_id id) :
	QGLWidget(QGLFormat(QGL::SingleBuffer), owner),
	render_thread(new RenderThread(this)),
	id(id),
	_what(IDLE),
	_gifmovie_filepath(nullptr),
	_screenshot_filepath(nullptr),
	frames_hit_percent(100.0f),
	zoom(calc_zoom()),
	screen_renderer(newRenderer()),
	gif_writer(nullptr)
{
	xlogIn("new Screen");

	setAttribute(Qt::WA_OpaquePaintEvent, 1); // we paint all pixels
	// setAttribute(Qt::WA_NoSystemBackground,1);	// the widget has transparent parts
	setAutoFillBackground(false); // else ctors of Painters in paint_screen() will erase whole Screen

	assert(context()->isValid());
	doneCurrent(); // release OGL context so that render_thread can aquire it

#if QT_VERSION >= 0x050000
	context()->moveToThread(render_thread);
	xlogline("moved context to render_thread");
#endif

	render_thread->start(); // start default run() which calls exec() to run the event loop
}

Screen::~Screen()
{
	xlogIn("~Screen");

	_mutex.lock();
	_what |= TERMI;
	_mutex.unlock();
	_sema.release();

	render_thread->wait();

	delete gif_writer;
	delete[] _gifmovie_filepath;
	delete[] _screenshot_filepath;
}

void Screen::initializeGL()
{
	// Setup resources needed by the OpenGL implementation to render the scene.
	QGLWidget::initializeGL();
}

GifWriter* Screen::newGifWriter(bool update_border, uint fps)
{
	switch (uint(id))
	{
	case isa_ScreenTc2048: return new Tc2048GifWriter(update_border, fps);
	case isa_ScreenZxsp: return new ZxspGifWriter(update_border, fps);
	case isa_ScreenSpectra: return new SpectraGifWriter(update_border, fps);
	case isa_ScreenMono: return new MonoGifWriter(update_border, fps);
	default: IERR();
	}
}

Renderer* Screen::newRenderer()
{
	// Instances of Renderer can't be returned by a overloaded function,
	// because we call this in the constructor!

	switch (uint(id))
	{
	case isa_ScreenTc2048: return new Tc2048Renderer();
	case isa_ScreenZxsp: return new ZxspRenderer();
	case isa_ScreenSpectra: return new SpectraRenderer();
	case isa_ScreenMono: return new MonoRenderer();
	default: IERR();
	}
}

void Screen::setFlavour(isa_id new_id)
{
	// set screen renderer
	// note: we want to replace a zxsp screen with a Spectra screen
	// without actually replacing the current screen widget.
	// therefore ScreenZxsp must be able to mogrify into any of it's 'flavours'.
	// subclassing does not work, because it would require to create a new instance of the new class.

	if (isA(isa_ScreenZxsp))
		assert(new_id == isa_ScreenZxsp || new_id == isa_ScreenTc2048 || new_id == isa_ScreenSpectra);
	if (isA(isa_ScreenMono)) assert(new_id == isa_ScreenMono);

	if (this->id == new_id) return;

	this->id = new_id;

	_mutex.lock();
	delete screen_renderer;
	screen_renderer = newRenderer();
	_mutex.unlock();
}


// =========================================================================
//							Methods
// =========================================================================

void Screen::paintGL() { abort("Screen::paintGL() called!"); }

void Screen::resizeGL(int, int) { abort("Screen::resizeGL(int,int) called!"); }

void Screen::paintEvent(QPaintEvent*)
{
	repaint(); //	QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::resizeEvent(QResizeEvent*)
{
	calc_zoom();
	repaint(); //	QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::repaint()
{
	// store a repaint request.
	// to be used in paintGL(), resizeGL(w,h), paintEvent() and resizeEvent().
	// the method waits until the render thread has painted the entire screen.

	_mutex.lock();

	assert(!(_what & REPAINT));
	assert(_wait_repaint_sema.available() == 0);
	//_wait_repaint_sema.acquire(_wait_repaint_sema.available());

	_what |= REPAINT;

	_mutex.unlock();

	_sema.release();
	_wait_repaint_sema.acquire();
}

void Screen::saveScreenshot(cstr path)
{
	if (_screenshot_filepath) return;
	_screenshot_filepath = newcopy(path);
}

void Screen::startRecording(cstr path, bool with_border)
{
	if (_gifmovie_filepath) return;
	_gifmovie_with_bordereffects = with_border;
	_gifmovie_filepath			 = newcopy(path);
}

void Screen::stopRecording()
{
	_mutex.lock();
	_what |= STOPMOVIE;
	_mutex.unlock();
	_sema.release();
}

void Screen::do_render_thread()
{
	// run the event loop of the render thread.
	// waits for requests and executes them.
	// possible requests are: TERMI, REPAINT and FFB

	try
	{
		for (;;)
		{
			_sema.acquire();
			if (_what & TERMI) break;
			_mutex.lock();

			if (_what & FFB_OR_VBI)
			{
				do_ffb_or_vbi(); // must unlock mutex after parameter transfer

				_mutex.lock();
				_what &= ~FFB_OR_VBI;
				_mutex.unlock();
				continue;
			}

			if (_what & STOPMOVIE)
			{
				_what &= ~STOPMOVIE;
				_mutex.unlock();

				if (gif_writer) try
					{
						gif_writer->stopRecording();
					}
					catch (FileError& e)
					{
						showWarning("File error: %s", e.what());
					}
				delete gif_writer;
				gif_writer = nullptr;
				continue;
			}

			if (_what & REPAINT)
			{
				_what &= ~REPAINT;
				_mutex.unlock();

				if (isVisible()) paint_screen(yes);
				_wait_repaint_sema.release();
				continue;
			}

			xlogline("Screen::do_render_thread: unknown command");
			for (uint m = 16; m; m += m)
			{
				if (_what & m)
				{
					_what -= m;
					break;
				}
			} // reset one bit
			_mutex.unlock();
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

void Screen::paint_screen(bool draw_passepartout)
{
	makeCurrent();

	int hf = (screen_renderer->width - 2 * screen_renderer->h_border) / 256; // hor. stretch factor 256 -> 512

	// setup geometry
	int zoom	 = minmax(1, min(width() / 256, height() / 192), 4); // getZoom()
	int w		 = (width() + zoom - 1) / zoom;						 // window size
	int h		 = (height() + zoom - 1) / zoom;
	int x		 = (w + 1 - 256) / 2; // position of screenfile in this widget
	int y		 = (h + 1 - 192) / 2;
	int v_border = min(y, V_BORDER_MAX);
	int h_border = min(x, H_BORDER_MAX);
	int h_black	 = x - h_border;
	int v_black	 = y - v_border;

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

	// draw passpartout, if required:
	//	note on cpu usage:							zoom=2	fullscreen		(oGL only, no Painter)
	//	passepartout only drawn when requested		8.1%	7.5%			approx.
	//	passepartout always drawn, border only		10.6%	16.6%			approx.
	//	passepartout always drawn, full rect		18.5%	20.2%			approx.
	//
	//
	//	note on overlays:
	//	transparent pixels draw opaque, because the underlying pixels were not drawn (opacity 0%)
	//	Mist...
	//
	if (draw_passepartout && (v_black > 0 || h_black > 0))
	{
		static const int screen_h = 192 + 2 * V_BORDER_MAX;
		static const int screen_w = 256 + 2 * H_BORDER_MAX;

		if (v_black > 0)
		{
			draw_rect(0, 0, width(), zoom * v_black, black);
			draw_rect(0, zoom * v_black + zoom * screen_h, width(), zoom * v_black, black);
		}
		if (h_black > 0)
		{
			draw_rect(0, zoom * v_black, zoom * h_black, zoom * screen_h, black);
			draw_rect(zoom * h_black + zoom * screen_w, zoom * v_black, zoom * h_black, zoom * screen_h, black);
		}
	}

	// setup new pixels unpacking, transfer, mapping & rasterization:
	int qsx = screen_renderer->h_border; // position of screenfile in screen_renderer.bits[]
	int qsy = screen_renderer->v_border;
	int qbx = qsx - h_border * hf; // position of visible rect in screen_renderer.bits[]
	int qby = qsy - v_border;

	glRasterPos2i(zoom * h_black, zoom * v_black); // window coordinates
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		   // if RGBA
	glPixelZoom(GLfloat(zoom) / hf, -zoom);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, screen_renderer->width); // number of pixels

	// enable blending, if image from screen renderer has transparency:		Denk…
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// note: glDrawPixels(w,h,format,type,data*)
	glDrawPixels(
		h_border * 2 * hf + 256 * hf, v_border * 2 + 192, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
		screen_renderer->bits + qbx + qby * screen_renderer->width);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	uint err = glGetError();
	if (err) logline("OpenGL error: $%04x", err);

	p.endNativePainting();

	p.setBackgroundMode(Qt::BGMode::TransparentMode);
	p.scale(zoom, zoom);

	if (auto* ov = rzx_overlay.get())
	{
		ov->x = w - ov->w;
		ov->y = h - ov->h;
		ov->draw(p, zoom);
	}

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

	// flush drawing to screen:
	// without Painter resize() was (nearly) flicker-free:
	//
	//	if(doubleBuffer()) swapBuffers();
	//	else glFlush();				// flush all buffered commands to the GPU
	//	//else glFinish();			// also blocks until done

	// doneCurrent();

	if (XLOG)
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
}

void Screen::setRzxOverlay(const RzxOverlayPtr& p)
{
	if (rzx_overlay == p) return;

	_mutex.lock();
	rzx_overlay = p;
	_mutex.unlock();
}

void Screen::setJoystickOverlay(uint index, const JoystickOverlayPtr& p)
{
	assert(index < NELEM(joystick_overlays));

	if (joystick_overlays[index] == p) return;

	_mutex.lock();
	joystick_overlays[index] = p;
	_mutex.unlock();
}

void Screen::setNumJoystickOverlays(uint n)
{
	_mutex.lock();
	while (n < NELEM(joystick_overlays))
	{
		joystick_overlays[n++].reset(); //
	}
	_mutex.unlock();
}

void Screen::removeAllOverlays()
{
	_mutex.lock();
	rzx_overlay.reset();
	for (uint i = 0; i < NELEM(joystick_overlays); i++)
	{
		joystick_overlays[i].reset(); //
	}
	_mutex.unlock();
}


} // namespace gui
