// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "VideoData.h"
#include "graphics/gif/GifEncoder.h"

namespace zxsp
{

using RgbaColor = uint32;					 // actually ARGB for Qt
extern const RgbaColor zxsp_rgba_colors[16]; // ZxspRenderer.cpp

// ARGB for Qt:
constexpr RgbaColor black		   = 0xff000000;
constexpr RgbaColor blue		   = 0xff0000CC;
constexpr RgbaColor red			   = 0xffCC0000;
constexpr RgbaColor magenta		   = 0xffCC00CC;
constexpr RgbaColor green		   = 0xff00CC00;
constexpr RgbaColor cyan		   = 0xff00CCCC;
constexpr RgbaColor yellow		   = 0xffCCCC00;
constexpr RgbaColor white		   = 0xffCCCCCC;
constexpr RgbaColor bright_black   = 0xff000000;
constexpr RgbaColor bright_blue	   = 0xff0000FF;
constexpr RgbaColor bright_red	   = 0xffFF0000;
constexpr RgbaColor bright_magenta = 0xffFF00FF;
constexpr RgbaColor bright_green   = 0xff00FF00;
constexpr RgbaColor bright_cyan	   = 0xff00FFFF;
constexpr RgbaColor bright_yellow  = 0xffFFFF00;
constexpr RgbaColor bright_white   = 0xffFFFFFF;
constexpr RgbaColor grey		   = 0xff808080;


// ===========================================================
//					screen renderer:
// ===========================================================


template<typename Color>
class VideoFrame
{
public:
	int	   max_width  = 0; // max. videoframe width
	int	   max_height = 0; // max. videoframe height
	Color* pixels	  = nullptr;

	// filled in by Renderer:
	int				hf = 1;				 // hor. stretch factor: hf=2 for 64 char mode
	Size			frame {0, 0};		 // actual size of frame
	Rect			screen {0, 0, 0, 0}; // size & position of screen
	const Colormap* cmap	 = nullptr;	 // used by GifRecorder
	bool			flashing = no;

	VideoFrame() = default;
	VideoFrame(int max_w, int max_h, int hf = 1) :
		max_width(max_w),
		max_height(max_h),
		pixels(new Color[max_w * (max_h + 1)]),
		hf(hf),
		frame(max_w, max_h),
		screen((max_w - 256 * hf) / 2, (max_h - 192) / 2, 256 * hf, 192)
	{}
	~VideoFrame() { delete[] pixels; }

	void resize(int max_w, int max_h, int hf = 1)
	{
		delete[] pixels;
		new (this) VideoFrame(max_w, max_h, hf);
	}

	int frameWidth() const { return frame.width; }
	int frameHeight() const { return frame.height; }
	int screenWidth() const { return screen.width(); }
	int screenHeight() const { return screen.height(); }
	int topBorder() const { return screen.top(); }
	int leftBorder() const { return screen.left(); }
	int rightBorder() const { return frame.width - screen.width() - leftBorder(); }
	int bottomBorder() const { return frame.height - screen.height() - topBorder(); }
};


// create VideoFrame for display:

template<typename T>
void zx80Renderer(VideoFrame<T>*, VideoData*);
template<typename T>
void zxspRenderer(VideoFrame<T>*, VideoData*);
template<typename T>
void tc2048Renderer(VideoFrame<T>*, VideoData*);
template<typename T>
void spectraRenderer(VideoFrame<T>*, VideoData*);

extern template void zx80Renderer(VideoFrame<uint8>*, VideoData*);
extern template void zx80Renderer(VideoFrame<RgbaColor>*, VideoData*);
extern template void zxspRenderer(VideoFrame<uint8>*, VideoData*);
extern template void zxspRenderer(VideoFrame<RgbaColor>*, VideoData*);
extern template void tc2048Renderer(VideoFrame<uint8>*, VideoData*);
extern template void tc2048Renderer(VideoFrame<RgbaColor>*, VideoData*);
extern template void spectraRenderer(VideoFrame<uint8>*, VideoData*);
extern template void spectraRenderer(VideoFrame<RgbaColor>*, VideoData*);

} // namespace zxsp


/*
































*/
