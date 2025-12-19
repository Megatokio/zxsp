// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "GifRecorder.h"
#include "Interfaces/IScreen.h"
#include "Item.h"
#include "Overlays/Overlay.h"
#include "Renderer.h"
#include "Templates/Queue.h"
#include "Video/FrameData.h"
#include "graphics/geometry.h"
#include "zxsp_types.h"
#include <QGLWidget>
#include <QMutex>
#include <QSemaphore>
#include <QThread>


namespace zxsp
{
using coord			 = zxsp::coord;
using Point			 = zxsp::Point;
using Size			 = zxsp::Size;
using Dist			 = zxsp::Dist;
using Rect			 = zxsp::Rect;
using FrameData		 = zxsp::FrameData;
using FrameDataQueue = zxsp::FrameDataQueue;


class Screen : public QGLWidget
{
	NO_COPY_MOVE(Screen);
	friend class RenderThread;
	using VideoFrame  = zxsp::VideoFrame<RgbaColor>;
	using GifRecorder = zxsp::GifRecorder;

public:
	Screen(QWidget* owner, const Size& fb_size);
	~Screen() override;

	VideoFrame&		getCurrentFrame() { return current_frame; }
	FrameDataQueue* getFrameDataInQueue() { return &in_queue; }
	void			setFrameDataOutQueue(FrameDataQueue* q) { out_queue = q; }

	int	 getZoom() const { return zoom; /*minmax(1, min(width()/256, height()/192), 4); */ }
	bool isActive() const { return windowState() & Qt::WindowActive; }

	void repaint();

	void saveScreenshot(cstr path);
	void startRecording(cstr path, bool update_border);
	void stopRecording();
	bool isRecording() const { return gif_recorder != nullptr; }
	uint getFramesHit() const { return uint(frames_hit_percent + 0.5f); }
	int	 getLeftBorder() const { return ((width() + zoom - 1) / zoom + 1 - 256) / 2 * zoom; }
	int	 getTopBorder() const { return ((height() + zoom - 1) / zoom + 1 - 192) / 2 * zoom; }
	int	 getHF() const { return current_frame.screenWidth() / 256; }

	using RzxOverlayPtr		 = RCPtr<RzxOverlay>;
	using JoystickOverlayPtr = RCPtr<JoystickOverlay>;

	RzxOverlayPtr	   rzx_overlay;
	JoystickOverlayPtr joystick_overlays[4];
	void			   setRzxOverlay(const RzxOverlayPtr&);
	void			   setJoystickOverlay(uint index, const JoystickOverlayPtr&);
	void			   setNumJoystickOverlays(uint);
	void			   removeAllOverlays();

protected:
	// the queues do NOT take ownership of the objects:
	VideoFrame		current_frame;
	FrameDataQueue	in_queue;
	FrameDataQueue* out_queue {nullptr};
	QThread*		thread {nullptr};
	bool			termi	 = false;
	bool			_repaint = false;
	QSemaphore		wait_repaint_sema; // TODO: nötig?
	float			frames_hit_percent {100.0f};
	int				zoom;
	GifRecorder*	gif_recorder {nullptr};
	QMutex			mutex; // for overlays

	void do_render_thread();
	void draw_rect(int x, int y, int w, int h, RgbaColor color);
	void draw_overlays(QPainter&, int zoom);
	int	 calc_zoom() { return zoom = minmax(1, min(width() / 256, height() / 192), 4); }
	void do_draw_screen(bool draw_passepartout = yes);

	void paintGL() override;				  // Qt reimplement
	void resizeGL(int, int) override;		  // Qt reimplement
	void paintEvent(QPaintEvent*) override;	  // Qt reimplement
	void resizeEvent(QResizeEvent*) override; // Qt reimplement
	void initializeGL() override;			  // Qt reimplement
};

} // namespace zxsp


/*






















*/
