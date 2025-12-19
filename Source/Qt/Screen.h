// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "GifRecorder.h"
#include "Item.h"
#include "Machine.h"
#include "Overlays/Overlay.h"
#include "Renderer.h"
#include "Templates/Queue.h"
#include "Video/VideoData.h"
#include <QGLWidget>
#include <QMutex>
#include <QThread>

namespace zxsp
{

class Screen : public QGLWidget, public IScreen
{
	NO_COPY_MOVE(Screen);
	friend class ScreenUpdateThread;
	using VideoFrame = zxsp::VideoFrame<RgbaColor>;

public:
	explicit Screen(QWidget* owner);
	~Screen() override;

	VideoFrame& getCurrentFrame() { return current_frame; }
	int			getZoom() const { return zoom; }
	bool		isActive() const { return windowState() & Qt::WindowActive; }

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

	QMutex			   overlay_mutex;
	RzxOverlayPtr	   rzx_overlay;
	JoystickOverlayPtr joystick_overlays[4];
	void			   setRzxOverlay(const RzxOverlayPtr&);
	void			   setJoystickOverlay(uint index, const JoystickOverlayPtr&);
	void			   setNumJoystickOverlays(uint);
	void			   removeAllOverlays();

private:
	VideoFrame	 current_frame;
	GifRecorder* gif_recorder		= nullptr;
	float		 frames_hit_percent = 100.0f;
	int			 zoom				= 1;

	void do_render_thread();
	void calc_zoom() { zoom = minmax(1, min(width() / 256, height() / 192), 4); }
	void do_draw_screen();

	void paintGL() override;				  // Qt reimplement
	void resizeGL(int, int) override;		  // Qt reimplement
	void paintEvent(QPaintEvent*) override;	  // Qt reimplement
	void resizeEvent(QResizeEvent*) override; // Qt reimplement
	void initializeGL() override;			  // Qt reimplement
};

} // namespace zxsp


/*






















*/
