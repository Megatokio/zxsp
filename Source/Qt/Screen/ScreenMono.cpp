// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "graphics/gif/GifEncoder.h"
#include "ScreenMono.h"
#include "Ula/UlaMono.h"
#include "globals.h"
#include "cpp/cppthreads.h"
#include "MonoRenderer.h"


// colors & pixels:
static const uint16 i2r[2]={0,0xffff};
static const uint16 i2g[2]={0,0xffff};
static const uint16 i2b[2]={0,0xffff};


bool ScreenMono::ffb_or_vbi( uint8* new_pixels, int frame_w, int frame_h, int screen_w, int screen_h,
							 int x0, int y0, uint32 cc )
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
			_new_pixels = new_pixels;
			_frame_h = frame_h;
			_frame_w = frame_w;
			_screen_h = screen_h;
			_screen_w = screen_w;
			_screen_x0 = x0;
			_screen_y0 = y0;
			_cc = cc;
		}

	_mutex.unlock();

	if(ffb_ready) _sema.release();
	return ffb_ready;			// true --> retain new data for processing in progress, else still retain old data
}

bool ScreenMono::sendFrame(uint8* frame_data, const Size& frame_size, const Rect& screen)
{
	// store data for new FFB and trigger render thread.
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
			_new_pixels = frame_data;
			_frame_h = frame_size.height;
			_frame_w = frame_size.width;
			_screen_h = screen.height();
			_screen_w = screen.width();
			_screen_x0 = screen.p1.x;
			_screen_y0 = screen.p1.y;
			_cc = 0;
		}

	_mutex.unlock();

	if(ffb_ready) _sema.release();
	return ffb_ready;			// true --> retain new data for processing in progress, else still retain old data
}


void ScreenMono::do_ffb_or_vbi() noexcept(false)
{
	uint8* new_pixels = _new_pixels;
	uint frame_h = _frame_h;
	uint frame_w = _frame_w;
	uint screen_h = _screen_h;
	uint screen_w = _screen_w;
	uint screen_x0 = _screen_x0;
	uint screen_y0 = _screen_y0;
	uint32 cc = _cc;
	_mutex.unlock();

	if(isVisible())
	{
		MonoRendererPtr(screen_renderer)->drawScreen(
			new_pixels,screen_w,screen_h,frame_w,frame_h,screen_x0,screen_y0,cc);
		paint_screen(no);
	}

	if(_screenshot_filepath && cc==0)	// --> make a screenshot
	{
		cstr path = _screenshot_filepath;
		_screenshot_filepath = nullptr;

		MonoGifWriter* gif = new MonoGifWriter(this,no);
		try{ gif->saveScreenshot(path,new_pixels,screen_w,screen_h,frame_w,frame_h,screen_x0,screen_y0); }
		catch(FileError& e){ showWarning("File error: %s",e.what()); }
		delete[] path;
		delete gif;
	}

	if(_gifmovie_filepath && cc==0)		// --> start recording into an animated gif
	{
		cstr path = _gifmovie_filepath;
		bool with_border = _gifmovie_with_bordereffects;
		_gifmovie_filepath = nullptr;

		gif_writer = new MonoGifWriter(this,with_border);
		try
		{
			gif_writer->startRecording(path);
		}
		catch(FileError& e)
		{
			showWarning("File error: %s",e.what());
			delete gif_writer;
			gif_writer = nullptr;
		}
		delete[] path;
	}

	if(gif_writer && cc==0)				// --> record frame into gif movie file
	{
		try
		{
			MonoGifWriterPtr(gif_writer)->writeFrame(
				new_pixels,screen_w,screen_h,frame_w,frame_h,screen_x0,screen_y0);
		}
		catch(FileError& e)
		{
			showWarning("File error: %s",e.what());
			delete gif_writer;
			gif_writer = nullptr;
		}
	}
}

void ScreenMono::paint_screen(bool draw_passepartout)
{
	makeCurrent();

// setup geometry
	int zoom = minmax(1, min(width()/256, height()/192), 4);

	int w = (width()+zoom-1)/zoom;		// window size
	int h = (height()+zoom-1)/zoom;
	int x = (w+1-256) / 2;				// position of screenfile in this widget
	int y = (h+1-192) / 2;

	int v_border = min(y,V_BORDER_MAX);
	int h_border = min(x,H_BORDER_MAX);
	int h_black  = x-h_border;
	int v_black  = y-v_border;

// update viewport ((imageable area))
// glViewport specifies the affine transformation of x and y from normalized device coordinates to window coordinates.
	glViewport(0, 0, width(), height());

// setup indexed color conversion maps
	glPixelMapusv(GL_PIXEL_MAP_I_TO_R,2,i2r);
	glPixelMapusv(GL_PIXEL_MAP_I_TO_G,2,i2g);
	glPixelMapusv(GL_PIXEL_MAP_I_TO_B,2,i2b);

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

// setup new pixels unpacking, transfer, mapping & rasterization
	MonoRenderer* mono_screen_renderer = MonoRendererPtr(screen_renderer);
	int qsx = mono_screen_renderer->h_border;		// position of screenfile in screen_renderer.bits[]
	int qsy = mono_screen_renderer->v_border;
	int qbx = qsx - h_border;					// position of visible rect in screen_renderer.bits[]
	int qby = qsy - v_border;

	glRasterPos2i(zoom*h_black,zoom*v_black);	// window coordinates
	glPixelZoom(zoom, -zoom);
	//glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH,screen_renderer->width);		// number of pixels
	glPixelStorei( GL_UNPACK_ROW_LENGTH, mono_screen_renderer->width );
	// note: glDrawPixels(w,h,format,type,data*)
	glDrawPixels(h_border*2+256,v_border*2+192, GL_COLOR_INDEX, GL_BITMAP,
				 screen_renderer->mono_octets+(qbx+qby*screen_renderer->width)/8);

	if(draw_passepartout && (v_black|h_black))
	{
		const int screen_h = 192+2*V_BORDER_MAX;	// zxsp_screen+border total height
		const int screen_w = 256+2*H_BORDER_MAX;	// zxsp_screen+border total width

		draw_rect(0,0,width(),zoom*v_black,black);
		draw_rect(0,zoom*v_black,zoom*h_black,zoom*screen_h,black);
		draw_rect(zoom*h_black+zoom*screen_w,zoom*v_black,zoom*h_black,zoom*screen_h,black);
		draw_rect(0,zoom*v_black+zoom*screen_h,width(),zoom*v_black,black);
	}


// flush drawing to screen:
	if(doubleBuffer())
		swapBuffers();
	else glFlush();					// force actual execution of all buffered commands
//	else glFinish();				// also blocks until done

	uint err = glGetError();			// note: nach stopFullscreen() gibt's häufig einen Fehler!
	if(err) logline("OpenGL error: $%04x", err);

	doneCurrent();
}















































