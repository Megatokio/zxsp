// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Renderer.h"
#include "VideoData.h"

namespace zxsp
{

struct GifWriter : public VideoDataReceiver
{
	// the GifWriter unlinks & deletes itself once the screenshot is made
	GifWriter(cstr path);
	static void saveScreenshot(cstr path, const VideoData*);
};

class GifRecorder : public VideoDataReceiver
{
	friend class GifRenderThread;
	NO_COPY_MOVE(GifRecorder);

public:
	using VideoFrame = zxsp::VideoFrame<uint8>;

	GifRecorder(cstr path, bool update_border, int frames_per_second);
	~GifRecorder();
	void stopRecording();

	const Colormap* global_colormap = nullptr;
	const Size		frame {};  // video frame incl. border
	const Rect		screen {}; // position & size of screen file

private:
	void start_recording(cstr path, bool update_border, int frames_per_second);
	void do_render_thread();
	void write_diff2_to_file();
	void writeFrame(const VideoData*);

	int		   frame_count	 = 0;		   // for bits2
	Pixelmap*  bits			 = nullptr;	   // new screen
	Pixelmap*  diff			 = nullptr;	   // provided for diff
	Pixelmap*  bits2		 = nullptr;	   // old screen, not yet written to file
	Pixelmap*  diff2		 = nullptr;	   // old screen, not yet written to file, diff image to what is already in file
	bool	   update_border = false;	   // auch mit border animation?
	int		   frames_per_second	 = 50; // animation speed
	int		   frames_per_flashphase = 16; // for screenshot
	GifEncoder gif_encoder;
};

} // namespace zxsp
