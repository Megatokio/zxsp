// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 0
#include <QGLWidget>
#include <QSemaphore>
#include <QMutex>
#include <QThread>
#include <QGLFormat>
#include <QApplication>
#include "Machine.h"
#include "MachineController.h"
#include "Screen.h"
#include "Tc2048Renderer.h"
#include "ZxspRenderer.h"
#include "MonoRenderer.h"
#include "SpectraRenderer.h"
#include "Overlays/Overlay.h"
#include "unix/os_utilities.h"
#include "IsaObject.h"


// =========================================================================
//							Render Thread
// =========================================================================

class RenderThread : public QThread
{
	Screen* screen;
	void run() override { screen->do_render_thread(); }		// started by QThread::start()
public:
	explicit RenderThread(Screen* screen)					:QThread(screen),screen(screen){}
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

Screen::Screen (QWidget* owner, isa_id id)
:
	QGLWidget(QGLFormat(QGL::SingleBuffer),owner),
	render_thread(new RenderThread(this)),
	id(id),
	_what(IDLE),
	_gifmovie_filepath(NULL),
	_screenshot_filepath(NULL),
	frames_hit_percent(100.0f),
	zoom(calc_zoom()),
	screen_renderer(newRenderer()),
	gif_writer(NULL),
	overlays{nullptr,nullptr}
{
	xlogIn("new Screen");

	setAttribute(Qt::WA_OpaquePaintEvent,1);		// we paint all pixels
	//setAttribute(Qt::WA_NoSystemBackground,1);	// the widget has transparent parts
	setAutoFillBackground(false);	// else ctors of Painters in paint_screen() will erase whole Screen

	assert(context()->isValid());
	doneCurrent();					// release OGL context so that render_thread can aquire it

	#if QT_VERSION >= 0x050000
	  context()->moveToThread(render_thread);
	  xlogline("moved context to render_thread");
	#endif

	render_thread->start();			// start default run() which calls exec() to run the event loop
}

Screen::~Screen()
{
	xlogIn("~Screen");

	_mutex.lock();
	_what |= TERMI;
	_mutex.unlock();
	_sema.release();

	render_thread->wait();

	for(uint i=NELEM(overlays);i--;) delete overlays[i];
	delete gif_writer;
	delete _gifmovie_filepath;
	delete _screenshot_filepath;
}

void Screen::initializeGL()
{
	// Setup resources needed by the OpenGL implementation to render the scene.
	QGLWidget::initializeGL();
}

GifWriter* Screen::newGifWriter(bool update_border)
{
	switch(uint(id))
	{
	case isa_ScreenTc2048:	return new Tc2048GifWriter(this,update_border);
	case isa_ScreenZxsp:	return new ZxspGifWriter(this,update_border);
	case isa_ScreenSpectra:	return new SpectraGifWriter(this,update_border);
	case isa_ScreenMono:	return new MonoGifWriter(this,update_border);
	default: IERR();
	}
}

Renderer* Screen::newRenderer()
{
	// Instances of Renderer can't be returned by a overloaded function,
	// because we call this in the constructor!

	switch(uint(id))
	{
	case isa_ScreenTc2048:	return new Tc2048Renderer(this);
	case isa_ScreenZxsp:	return new ZxspRenderer(this);
	case isa_ScreenSpectra:	return new SpectraRenderer(this);
	case isa_ScreenMono:	return new MonoRenderer(this);
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

	if(isA(isa_ScreenZxsp)) assert(new_id==isa_ScreenZxsp || new_id==isa_ScreenTc2048 || new_id==isa_ScreenSpectra);
	if(isA(isa_ScreenMono)) assert(new_id==isa_ScreenMono);

	if(this->id == new_id) return;

	this->id = new_id;

	_mutex.lock();
		delete screen_renderer;
		screen_renderer = newRenderer();
	_mutex.unlock();
}


// =========================================================================
//							Methods
// =========================================================================

void Screen::paintGL()
{
	abort("Screen::paintGL() called!");
}

void Screen::resizeGL(int,int)
{
	abort("Screen::resizeGL(int,int) called!");
}

void Screen::paintEvent(QPaintEvent*)
{
	repaint();	//	QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

void Screen::resizeEvent(QResizeEvent*)
{
	int z = zoom;
	calc_zoom();
	if(z!=zoom) for(uint i=NELEM(overlays); i--;)
	{
		Overlay* ov = overlays[i]; if(ov) ov->setZoom(zoom);
	}
	arrangeOverlays();
	repaint();	//	QGLWidget::paintEvent(e);	MUST NOT BE CALLED!
}

bool ScreenZxsp::ffb_or_vbi( IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels,
							 uint32 cc_start_of_screenfile, uint cc_per_scanline,
							 bool flashphase, uint32 cc )
{
	// store data for new FFB and trigger render thread.
	// the arrays ioinfo[] and attr_pixels[] are managed by the caller.
	// they are never deleted by this Screen or RenderThread.
	// returns true  if new buffers must be retained and old buffers may now be reused.
	// returns false if new buffers may be reused and old buffers must remain retained.

	_mutex.lock();

		bool ffb_ready = ~_what & FFB_OR_VBI;
		frames_hit_percent *= 0.98f;
		if(ffb_ready)
		{
			frames_hit_percent += 2.0f;

			_what |= FFB_OR_VBI;
			_ioinfo = ioinfo;
			_ioinfo_count = ioinfo_count;
			_attrpixels = attr_pixels;
			_cc_start_of_screenfile = cc_start_of_screenfile;
			_cc_per_scanline = cc_per_scanline;
			_cc = cc;
			_flashphase = flashphase;
		}

	_mutex.unlock();

	if(ffb_ready) _sema.release();
	return ffb_ready;			// true --> retain new data for processing in progress, else still retain old data
}

void Screen::repaint()
{
	// store a repaint request.
	// to be used in paintGL(), resizeGL(w,h), paintEvent() and resizeEvent().
	// the method waits until the render thread has painted the entire screen.

	_mutex.lock();

		assert( !(_what & REPAINT) );
		assert(_wait_repaint_sema.available()==0);
		//_wait_repaint_sema.acquire(_wait_repaint_sema.available());

		_what |= REPAINT;

	_mutex.unlock();

	_sema.release();
	_wait_repaint_sema.acquire();
}

void Screen::saveScreenshot(cstr path) throws
{
	if(_screenshot_filepath) return;
	_screenshot_filepath = newcopy(path);
}

void Screen::startRecording(cstr path, bool with_border) throws
{
	if(_gifmovie_filepath) return;
	_gifmovie_with_bordereffects = with_border;
	_gifmovie_filepath = newcopy(path);
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
		for(;;)
		{
			_sema.acquire();
			if(_what & TERMI) break;
			_mutex.lock();

			if(_what&FFB_OR_VBI)
			{
				do_ffb_or_vbi();		// must unlock mutex after parameter transfer

				_mutex.lock();
					_what &= ~FFB_OR_VBI;
				_mutex.unlock();
				continue;
			}

			if(_what&STOPMOVIE)
			{
				_what &= ~STOPMOVIE;
				_mutex.unlock();

				if(gif_writer)
				try{ gif_writer->stopRecording(); }
				catch(FileError& e){ showWarning("File error: %s",e.what()); }
				delete gif_writer; gif_writer = NULL;
				continue;
			}

			if(_what&REPAINT)
			{
				_what &= ~REPAINT;
				_mutex.unlock();

				if(isVisible()) paint_screen(yes);
				_wait_repaint_sema.release();
				continue;
			}

			xlogline("Screen::do_render_thread: unknown command");
			for(uint m=16;m;m+=m) { if(_what&m) { _what-=m; break; } }	// reset one bit
			_mutex.unlock();
		}
	}
	catch(std::exception& e)
	{
		showAlert("screen render thread crashed: %s",e.what());
	}

#if QT_VERSION >= 0x050000
	context()->moveToThread(QApplication::instance()->thread());
	xlogline("moved context back to gui thread");
#endif
}

void Screen::draw_rect(int x, int y, int w, int h, RgbaColor color)
{
	glRasterPos2i(x,y);							// window coordinates
	glPixelZoom(w,-h);
	glDrawPixels(1,1,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,&color);
}

void Screen::paint_screen(bool draw_passepartout)
{
	makeCurrent();

	int hf = (screen_renderer->width-2*screen_renderer->h_border)/256;		// hor. stretch factor 256 -> 512

	// setup geometry
	int zoom = minmax(1, min(width()/256, height()/192), 4);	// getZoom()
	int w = (width()+zoom-1)/zoom;		// window size
	int h = (height()+zoom-1)/zoom;
	int x = (w+1-256) / 2;				// position of screenfile in this widget
	int y = (h+1-192) / 2;
	int v_border = min(y,V_BORDER_MAX);
	int h_border = min(x,H_BORDER_MAX);
	int h_black  = x-h_border;
	int v_black  = y-v_border;

	// create painter (for drawing overlays) but first do native openGL painting:
	QPainter p(this);
	p.beginNativePainting();
	(void)glGetError();	// clear error

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
	glOrtho( 0,width(), height(),0, -1,1 );		// left, right, bottom, top, near_, far_clipping_plane

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
	if(draw_passepartout && (v_black>0||h_black>0))
	{
		static const int screen_h = 192 + 2 * V_BORDER_MAX;
		static const int screen_w = 256 + 2 * H_BORDER_MAX;

		if(v_black>0)
		{
			draw_rect(0,0,width(),zoom*v_black,black);
			draw_rect(0,zoom*v_black+zoom*screen_h,width(),zoom*v_black,black);
		}
		if(h_black>0)
		{
			draw_rect(0,zoom*v_black,zoom*h_black,zoom*screen_h,black);
			draw_rect(zoom*h_black+zoom*screen_w,zoom*v_black,zoom*h_black,zoom*screen_h,black);
		}
	}

	// setup new pixels unpacking, transfer, mapping & rasterization:
	int qsx = screen_renderer->h_border;		// position of screenfile in screen_renderer.bits[]
	int qsy = screen_renderer->v_border;
	int qbx = qsx - h_border*hf;				// position of visible rect in screen_renderer.bits[]
	int qby = qsy - v_border;

	glRasterPos2i(zoom*h_black, zoom*v_black);	// window coordinates
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		// if RGBA
	glPixelZoom(GLfloat(zoom)/hf, -zoom);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, screen_renderer->width);	// number of pixels

	// enable blending, if image from screen renderer has transparency:		Denk…
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// note: glDrawPixels(w,h,format,type,data*)
	glDrawPixels( h_border*2*hf+256*hf, v_border*2+192, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				  screen_renderer->bits + qbx + qby*screen_renderer->width );

	glPixelStorei(GL_UNPACK_ROW_LENGTH,0);

	uint err = glGetError();
	if(err) logline("OpenGL error: $%04x", err);

	p.endNativePainting();

	OverlayPlay* ovp = overlayPlay;
	if(ovp) p.drawPixmap(ovp->x, ovp->y, ovp->w, ovp->h, ovp->background);

	OverlayRecord* ovr = overlayRecord;
	if(ovr) p.drawPixmap(ovr->x, ovr->y, ovr->w, ovr->h, ovr->background);

	// flush drawing to screen:
	// without Painter resize() was (nearly) flicker-free:
	//
	//	if(doubleBuffer()) swapBuffers();
	//	else glFlush();				// flush all buffered commands to the GPU
	//	//else glFinish();			// also blocks until done

	//doneCurrent();

	if(XLOG)
	{
		static int n;
		if((++n&0xFF)==0)
		{
			double loads[3];
			sysLoad(loads);

			static double average = 0.1;
			double load = cpuLoad();
			average = average * 0.9 + load * 0.1;
			logline("sysLoad = %1.3f, cpuLoad = %.2f%% (avg = %.2f%%)", loads[0], load*100, average*100);
		}
	}
}

void ScreenZxsp::do_ffb_or_vbi() noexcept(false) // std::exception
{
	uint8*	attrpixels = _attrpixels;
	IoInfo*	ioinfo = _ioinfo;
	uint	ioinfo_count = _ioinfo_count;
	uint	cc_per_scanline = _cc_per_scanline;
	uint32	cc_start_of_screenfile = _cc_start_of_screenfile;
	uint32	cc = _cc;
	bool	flashphase = _flashphase;
	_mutex.unlock();

	if(isVisible())				// --> draw screen
	{
		ZxspRendererPtr(screen_renderer)->drawScreen(
			ioinfo, ioinfo_count, attrpixels, cc_per_scanline, cc_start_of_screenfile, flashphase, cc );
		paint_screen(no);
	}

	if(_screenshot_filepath)	// --> make a screenshot
	{
		cstr path = _screenshot_filepath;
		_screenshot_filepath = NULL;

		ZxspGifWriter* gif = static_cast<ZxspGifWriter*>(newGifWriter(no));
		try
		{
			gif->saveScreenshot(path,ioinfo,ioinfo_count,attrpixels,cc_per_scanline,cc_start_of_screenfile);
		}
		catch(FileError& e)
		{
			showWarning("File error: %s",e.what());
		}
		delete[] path;
		delete gif;
	}

	if(_gifmovie_filepath)		// --> start recording into an animated gif
	{
		cstr path = _gifmovie_filepath;
		bool with_border = _gifmovie_with_bordereffects;
		_gifmovie_filepath = NULL;

		gif_writer = ZxspGifWriterPtr(newGifWriter(with_border));
		try
		{
			gif_writer->startRecording(path);
		}
		catch(FileError& e)
		{
			showWarning("File error: %s",e.what());
			delete gif_writer;
			gif_writer = NULL;
		}
		delete[] path;
	}

	if(gif_writer)				// --> record frame into gif movie file
	{
		try
		{
			ZxspGifWriterPtr(gif_writer)->writeFrame(
				ioinfo, ioinfo_count, attrpixels, cc_per_scanline, cc_start_of_screenfile, flashphase);
		}
		catch(FileError& e)
		{
			showWarning("File error: %s",e.what());
			delete gif_writer;
			gif_writer = NULL;
		}
	}
}

void Screen::arrangeOverlays()
{
	//  Arrange Overlays
	//  Arranges Overlays according to their Position code:
	//
	//  TopLeft		TopCenter		TopRight
	//  MiddleLeft					MiddleRight
	//  BottomLeft	BottomCenter	BottomRight
	//  			BelowALL

	QRect bbox = this->rect();
	assert(bbox.top()==0);
	assert(bbox.left()==0);

	Overlay* ovs[NELEM(overlays)];
	uint num_ovs = 0;

	// collect Overlays
	// place BelowAll overlays
	// calc. max width / height of left / right / top / bottom Overlays
	// calc total width of top center and bottom center overlays
	// calc total height of left and right overlays

	int l=0,		// max. width of left overlays
		r=0,		// max. width of right overlays
		t=0,		// max. height of top overlays
		b=0;		// max. height of bottom overlays
	int tcw=0,		// total width: top center overlays
		bcw=0,		//				bottom center
		tlh=0,		// total height:top left
		trh=0,		//				top right
		blh=0,		//				bottom left
		brh=0,		//				bottom right
		lmh=0,		//				left middle
		rmh=0;		//				right middle

	for(uint i=NELEM(overlays); i--;)
	{
		Overlay* ov = overlays[i]; if(!ov) continue;

		switch(ov->position)
		{
		case Overlay::BelowAll:
			ov->x = bbox.left(); ov->y = bbox.bottom() - ov->h;
			ov->w = bbox.width();
			bbox.setHeight(bbox.height() - ov->h);
			continue;
		case Overlay::TopCenter:	t = max(t, ov->h); tcw += ov->w; break;
		case Overlay::BottomCenter:	b = max(b, ov->h); bcw += ov->w; break;
		case Overlay::TopLeft:		tlh += ov->h; goto bl;
		case Overlay::MiddleLeft:	lmh += ov->h; goto bl;
		case Overlay::BottomLeft:	blh += ov->h; bl: l = max(l, ov->w); break;
		case Overlay::TopRight:		trh += ov->h; goto br;
		case Overlay::MiddleRight:	rmh += ov->h; goto br;
		case Overlay::BottomRight:	brh += ov->h; br: r = max(r, ov->w); break;
		default: IERR();
		}
		ovs[num_ovs++] = ov;
	}

	// place Overlays:

	int tcx = (bbox.width() - tcw) / 2;		// x for placing TopCenter
	int bcx = (bbox.width() - bcw) / 2;		// x for placing BottomCenter

#define try _try

	int tly = 0;							// y for placing topLeft
	int try = 0;							// y for placing topRight
	if(tcx<l || tcx<r) tly = try = t;		// if topCenter too wide, start below topCenter

	int bly = bbox.bottom() - blh;			// y for placing bottomLeft
	int bry = bbox.bottom() - brh;			// y for placing bottomRight
	if(bcx<l || bcx<r) { bly -= b; bry -= b; }

	int lmy = (bbox.height() - lmh) / 2;	// y for placing middleLeft
		lmy = max(lmy, tly+tlh);

	int rmy = (bbox.height() - rmh) / 2;	// y for placing middleLeft
		rmy = max(rmy, try+trh);

	b = bbox.bottom()*2 - b;
	r = bbox.right()*2 - r;					// --> bbox.right() - ( r + ov->width()) / 2

	for(uint i=num_ovs; i--;)
	{
		Overlay* ov = ovs[i];
		switch(ov->position)
		{
		default: IERR();
		case Overlay::TopCenter:	ov->x = tcx; ov->y = (t - ov->h) / 2; tcx += ov->w; break;
		case Overlay::BottomCenter:	ov->x = bcx; ov->y = (b - ov->h) / 2; bcx += ov->w; break;
		case Overlay::TopLeft:		ov->x = (l - ov->w) / 2; ov->y = tly; tly += ov->h; break;
		case Overlay::MiddleLeft:	ov->x = (l - ov->w) / 2; ov->y = lmy; lmy += ov->h; break;
		case Overlay::BottomLeft:	ov->x = (l - ov->w) / 2; ov->y = bly; bly += ov->h; break;
		case Overlay::TopRight:		ov->x = (r - ov->w) / 2; ov->y = try; try += ov->h; break;
		case Overlay::MiddleRight:	ov->x = (r - ov->w) / 2; ov->y = rmy; rmy += ov->h; break;
		case Overlay::BottomRight:	ov->x = (r - ov->w) / 2; ov->y = bry; bry += ov->h; break;
		}
	}

#undef try
}

Overlay* Screen::findOverlay(isa_id id)
{
	for(uint i=NELEM(overlays);i--;)
	{
		if(overlays[i]->isA(id)) return overlays[i];
	}
	return nullptr;
}

void Screen::showOverlayPlay(bool f)
{
	if(!!overlayPlay==f) return;
	if(f) overlayPlay = new OverlayPlay(this);
	else { delete overlayPlay; overlayPlay = nullptr; }
	arrangeOverlays();
	update();				// falls das Passepartout nicht gezeichnet wird
}

void Screen::showOverlayRecord(bool f)
{
	if(!!overlayRecord==f) return;
	if(f) overlayRecord = new OverlayRecord(this);
	else { delete overlayRecord; overlayRecord = nullptr; }
	arrangeOverlays();
	update();				// falls das Passepartout nicht gezeichnet wird
}

void Screen::removeAllOverlays()
{
	for(uint i=NELEM(overlays);i--;) delete overlays[i];
	memset(overlays,0,sizeof(overlays));
	update();				// falls das Passepartout nicht gezeichnet wird
}






















































