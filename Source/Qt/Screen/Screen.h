/*	Copyright  (c)	Günter Woigk 2002 - 2018
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef SCREEN_H
#define SCREEN_H

#include <QGLWidget>
#include <QSemaphore>
#include <QMutex>
#include "zxsp_types.h"
#include "Item.h"
#include "Renderer/Tc2048Renderer.h"
#include "Overlays/Overlay.h"
struct IoInfo;
//typedef uint32	RgbaColor;			// RGBA


#define H_BORDER_MAX	64		// in 32 column mode
#define V_BORDER_MAX	48


class Screen : public QGLWidget
{
	class RenderThread*	render_thread;
	friend class RenderThread;

protected:
	isa_id		id;         // precise isa_id for this item
	//isa_id  	grp_id;     // major base class of this item = isa_Screen
	//cstr		name;

	QSemaphore	_sema;
	QMutex		_mutex;
	QSemaphore	_wait_repaint_sema;

	uint		_what;			enum { IDLE,FFB_OR_VBI=1,REPAINT=2,TERMI=4,STOPMOVIE=8 };
	cstr		_gifmovie_filepath;
	cstr		_screenshot_filepath;
	bool		_gifmovie_with_bordereffects;

	float		frames_hit_percent;

	int			zoom;

	Renderer*	screen_renderer;
	GifWriter*	gif_writer;

	void		do_render_thread();		// call from render_thread
	void		draw_rect		(int x, int y, int w, int h, RgbaColor color);
VIR	void		paint_screen	(bool draw_passepartout=yes);
	GifWriter*	newGifWriter	(bool update_border);
	Renderer*	newRenderer		();
	int			calc_zoom		()		{ return zoom = minmax(1, min(width()/256, height()/192), 4); }

protected:
VIR	void		do_ffb_or_vbi	()		noexcept(false) = 0; // std::exception // call from render_thread

	void		paintGL			();					// Qt reimplement
	void		resizeGL        (int,int);			// Qt reimplement
	void		paintEvent      (QPaintEvent*);		// Qt reimplement
	void		resizeEvent     (QResizeEvent*);	// Qt reimplement
	void		initializeGL    ();					// Qt reimplement

	Screen(QWidget* owner, isa_id);
	Screen(const Screen&) = delete;
	Screen& operator=(const Screen&) = delete;

public:
	virtual ~Screen();

	void		setFlavour		(isa_id);
	void		repaint			();

	void		saveScreenshot	(cstr path) throws;
	void		startRecording  (cstr path, bool update_border) throws;
	void		stopRecording	();
	bool		isRecording		() const			{ return _gifmovie_filepath != NULL; }

	bool		isA				(isa_id i) volatile const	{ isa_id j=id; while(j&&j!=i) { j=isa_pid[j]; } return i==j; }
								// { isa_id j=id; do{ if(i==j) return yes; }while((j=isa_pid[j])); return no; }

	int			getZoom			() const	{ return zoom; /*minmax(1, min(width()/256, height()/192), 4); */ }
	uint		getFramesHit    () const	{ return uint(frames_hit_percent+0.5f); }
	Renderer*	getScreenRenderer() const	{ return screen_renderer; }
	int			getLeftBorder	() const	{ return ((width() +zoom-1)/zoom+1-256) / 2 * zoom; }
	int			getTopBorder	() const	{ return ((height()+zoom-1)/zoom+1-192) / 2 * zoom; }
	int			getHF			() const	{ return (screen_renderer->width-2*screen_renderer->h_border)/256; }


public:
	union
	{
		Overlay*	overlays[2];
		struct
		{
			OverlayPlay	  *overlayPlay;
			OverlayRecord *overlayRecord;
		};
	};
	Overlay*	findOverlay(isa_id);
	void		arrangeOverlays();
	void		removeAllOverlays();
	void		showOverlayPlay(bool=true);
	void		hideOverlayPlay()				{ showOverlayPlay(false); }
	void		showOverlayRecord(bool=true);
	void		hideOverlayRecord()				{ showOverlayRecord(false); }
	void		addOverlay(Overlay*)			{ logline("Screen:addOverlay(): TODO"); }
	void		removeOverlay(Overlay*)			{ logline("Screen:removeOverlay(): TODO"); }
};



// ==========================================================
//			Sub Classes
// ==========================================================


class ScreenZxsp : public Screen
{
	uint8*	_attrpixels;
	IoInfo*	_ioinfo;
	uint	_ioinfo_count;
	uint	_cc_per_scanline;
	uint32	_cc_start_of_screenfile;
	uint32	_cc;
	bool	_flashphase;

protected:
	void	do_ffb_or_vbi() noexcept(false) override;

public:
	explicit ScreenZxsp(QWidget* owner, isa_id id=isa_ScreenZxsp) :Screen(owner,id){}
	ScreenZxsp(const ScreenZxsp&) = delete;
	ScreenZxsp& operator=(const ScreenZxsp&) = delete;

	bool ffb_or_vbi( IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint32 cc_start_of_screenfile,
					 uint cc_per_scanline, bool flashphase, uint32 cc );
};



// define safe casting procs:			e.g. Item* ItemPtr(object)

#define DEFSPTR(ITEM)	\
inline ITEM* ITEM ## Ptr(Screen*o) {					\
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<ITEM*>(o); }				\
\
inline const ITEM* ITEM ## Ptr(const Screen*o) {		\
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<const ITEM*>(o); }			\
\
inline volatile ITEM* ITEM ## Ptr(volatile Screen* o) { \
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<volatile ITEM*>(o); }		\
\
inline volatile const ITEM* ITEM ## Ptr(volatile const Screen* o) { \
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<volatile const ITEM*>(o); }	\

DEFSPTR(ScreenMono)
DEFSPTR(ScreenZxsp)


#endif



















