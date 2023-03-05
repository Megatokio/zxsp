#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Interfaces/IScreen.h"
#include "Item.h"
#include "Overlays/Overlay.h"
#include "Renderer.h"
#include "zxsp_types.h"
#include <QGLWidget>
#include <QMutex>
#include <QSemaphore>
struct IoInfo;


namespace gui
{

#define H_BORDER_MAX 64 // in 32 column mode
#define V_BORDER_MAX 48


class Screen : public QGLWidget, public IScreen
{
	class RenderThread* render_thread;
	friend class RenderThread;

protected:
	isa_id id; // precise isa_id for this item
	// isa_id  	grp_id;     // major base class of this item = isa_Screen
	// cstr		name;

	QSemaphore _sema;
	QMutex	   _mutex;
	QSemaphore _wait_repaint_sema;

	uint _what;
	enum { IDLE, FFB_OR_VBI = 1, REPAINT = 2, TERMI = 4, STOPMOVIE = 8 };
	cstr _gifmovie_filepath;
	cstr _screenshot_filepath;
	bool _gifmovie_with_bordereffects;

	float frames_hit_percent;

	int zoom;

	Renderer*  screen_renderer;
	GifWriter* gif_writer;

	void		 do_render_thread(); // call from render_thread
	void		 draw_rect(int x, int y, int w, int h, RgbaColor color);
	virtual void paint_screen(bool draw_passepartout = yes);
	GifWriter*	 newGifWriter(bool update_border, uint fps);
	Renderer*	 newRenderer();
	int			 calc_zoom() { return zoom = minmax(1, min(width() / 256, height() / 192), 4); }

protected:
	virtual void do_ffb_or_vbi() noexcept(false) = 0; // std::exception // call from render_thread

	void paintGL() override;				  // Qt reimplement
	void resizeGL(int, int) override;		  // Qt reimplement
	void paintEvent(QPaintEvent*) override;	  // Qt reimplement
	void resizeEvent(QResizeEvent*) override; // Qt reimplement
	void initializeGL() override;			  // Qt reimplement

	Screen(QWidget* owner, isa_id);
	Screen(const Screen&)			 = delete;
	Screen& operator=(const Screen&) = delete;

public:
	~Screen() override;

	// Interface IScreen:
	void setFlavour(isa_id) override;
	int	 getZoom() const { return zoom; /*minmax(1, min(width()/256, height()/192), 4); */ }
	void addOverlay(Overlay*) { logline("Screen:addOverlay(): TODO"); }
	void removeOverlay(Overlay*) { logline("Screen:removeOverlay(): TODO"); }
	bool isActive() const { return windowState() & Qt::WindowActive; }


	void repaint();

	void saveScreenshot(cstr path);
	void startRecording(cstr path, bool update_border);
	void stopRecording();
	bool isRecording() const { return _gifmovie_filepath != nullptr; }

	bool isA(isa_id i) const volatile
	{
		isa_id j = id;
		while (j && j != i) { j = isa_pid[j]; }
		return i == j;
	}

	uint	  getFramesHit() const { return uint(frames_hit_percent + 0.5f); }
	Renderer* getScreenRenderer() const { return screen_renderer; }
	int		  getLeftBorder() const { return ((width() + zoom - 1) / zoom + 1 - 256) / 2 * zoom; }
	int		  getTopBorder() const { return ((height() + zoom - 1) / zoom + 1 - 192) / 2 * zoom; }
	int		  getHF() const { return (screen_renderer->width - 2 * screen_renderer->h_border) / 256; }

	union
	{
		Overlay* overlays[2];
		struct
		{
			OverlayPlay*   overlayPlay;
			OverlayRecord* overlayRecord;
		};
	};
	Overlay* findOverlay(isa_id);
	void	 arrangeOverlays();
	void	 removeAllOverlays();
	void	 showOverlayPlay(bool = true);
	void	 hideOverlayPlay() { showOverlayPlay(false); }
	void	 showOverlayRecord(bool = true);
	void	 hideOverlayRecord() { showOverlayRecord(false); }
};

} // namespace gui


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
