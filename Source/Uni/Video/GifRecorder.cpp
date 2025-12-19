// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "GifRecorder.h"
#include "Templates/Array.h"
#include "unix/os_utilities.h"
#include "version.h"
#include <QSemaphore>
#include <QThread>


// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================


namespace zxsp
{

GifWriter::GifWriter(cstr path) { TODO(); }

class GifRenderThread : public QThread
{
	GifRecorder* recorder;
	virtual void run() override { recorder->do_render_thread(); }

public:
	explicit GifRenderThread(GifRecorder* r) : QThread(), recorder(r) {}
};

GifRecorder::GifRecorder(cstr path, bool update_border, int frames_per_second)
{
	start_recording(path, update_border, frames_per_second);
	start(new GifRenderThread(this));
}

GifRecorder::~GifRecorder() //
{
	stopRecording();
}

void GifRecorder::stopRecording()
{
	stop();

	if (gif_encoder.imageInProgress())
	{
		if (frame_count) write_diff2_to_file();
		gif_encoder.closeFile();
	}

	delete bits;
	bits = nullptr;
	delete diff;
	diff = nullptr;
	delete bits2;
	bits2 = nullptr;
	delete diff2;
	diff2 = nullptr;
}

void GifRecorder::do_render_thread()
{
	while (!_termi)
	{
		VideoData* data = getVideoData();
		if (data) writeFrame(data);
		// else termi
	}
}

void GifRecorder::start_recording(cstr path, bool update_border, int frames_per_second)
{
	assert(!gif_encoder.imageInProgress());
	assert(!bits && !bits2 && !diff && !diff2);

	this->update_border		= update_border;
	this->frames_per_second = frames_per_second;

	gif_encoder.openFile(path);
	gif_encoder.writeScreenDescriptor(frame.width, frame.height, 16);
	gif_encoder.writeCommentBlock(
		usingstr("created on %s by %s with %s %s\n", datestr(now()), getUser(), APPL_NAME, APPL_VERSION_STR));
	gif_encoder.writeLoopingAnimationExtension();

	frame_count = 0; // indicate first frame
	bits		= new Pixelmap(frame.width, frame.height);
	diff		= new Pixelmap(frame.width, frame.height);
	bits2		= new Pixelmap(frame.width, frame.height);
	diff2		= new Pixelmap(frame.width, frame.height);

	TODO(); // TODO: global_colormap, frame and screen
}

void GifRecorder::write_diff2_to_file()
{
	Colormap cmap = *global_colormap;
	diff2->reduceColors(cmap);
	int delay = (frame_count * 100 + frames_per_second / 2) / frames_per_second;
	gif_encoder.writeGraphicControlBlock(delay, cmap.transpColor());
	gif_encoder.writeImage(*diff2, cmap);
}

void GifRecorder::writeFrame(const VideoData*)
{
	TODO(); //
}

void GifWriter::saveScreenshot(cstr path, const VideoData*) // static
{
	TODO(); //
}

#if 0

ZxspGifWriter::ZxspGifWriter(bool update_border, uint frames_per_second) :
	GifWriter(isa_ZxspGifWriter, zxsp_colormap, 256, 192, 32, 24, update_border, frames_per_second)
{}

ZxspGifWriter::ZxspGifWriter(isa_id id, const Colormap& colormap, bool update_border, uint frames_per_second) :
	GifWriter(id, colormap, 256, 192, 32, 24, update_border, frames_per_second)
{}


void ZxspGifWriter::drawScreen(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase)
{
	assert((cc_start_of_screenfile & 3) == 0);
	if (!bits) bits = new Pixelmap(width, height);

	cc_start_of_screenfile += 4;

	uint32 cc_row_flyback			  = cc_per_scanline - cc_screen - 2 * cc_h_border;
	uint32 cc_start_of_visible_screen = cc_start_of_screenfile - cc_h_border - v_border * cc_per_scanline;
	uint32 cc_end_of_visible_screen	  = cc_start_of_visible_screen + height * cc_per_scanline - cc_row_flyback;

	// draw border:
	// and screenfile:

	GifColor bordercolor = 0; /*black*/

	uint32 cc_io		   = cc_start_of_visible_screen;
	ioinfo[ioinfo_count++] = IoInfo(cc_end_of_visible_screen, 0xfe, 0); // stopper
	GifColor* p			   = bits->getData();							// current pixel pointer
	GifColor* a			   = bits->getData();							// current start of row
	uint	  row		   = 0;											// current row in bits[]

	uint8* q = attr_pixels; // attr_pixels[] source pointer

	for (IoInfo* io = ioinfo; cc_io < cc_end_of_visible_screen; io++)
	{
		if (io->addr & 1) continue; // no ula address

		if (io->cc > cc_start_of_visible_screen)
		{
			uint32 cc = min(cc_end_of_visible_screen, (io->cc + 3) & ~3) - cc_start_of_visible_screen;

			uint end_row = cc / cc_per_scanline;
			uint end_col = min(uint(width), cc % cc_per_scanline * pixel_per_cc);
			assert(end_row < height);
			GifColor* ee = bits->getData() + end_row * width + end_col; // cc_io end pointer
			GifColor* e;												// intermediate ent pointers

			// draw all border pixels up to cc_io:
			while (p < ee)
			{
				if (row >= v_border && row < v_border + screen_height) // if inside screenfile region:
				{
					e = min(a + h_border, ee);
					while (p < e) { *p++ = bordercolor; } // draw left border
					if (p < a + h_border) break;		  // exit pixel loop if at cc_io

					// draw screen row:
					for (e += screen_width; p < e;)
					{
						uint pixels = *q++;
						uint attr	= *q++;

						if (attr & 0x80 && flashphase) pixels ^= 0xff;

						uint pen_color = (attr & 7) + ((attr >> 3) & 8);
						if (pen_color == transp) pen_color = 0;
						uint paper_color = (attr >> 3) & 15;
						if (paper_color == transp) paper_color = 0;

						for (uint m = 0x80; m; m = m >> 1) { *p++ = pixels & m ? pen_color : paper_color; }
					}
				}

				// draw (remainder of) screen row
				e = min(a + width, ee);
				while (p < e) { *p++ = bordercolor; }

				if (p == a + width)
				{
					a = p;
					row++;
				} // advance row number if end of row reached
			}
		}

		cc_io		= io->cc;
		bordercolor = io->byte & 7;
	}

	assert(p == bits->getData() + width * height);
}


/*	append frame to a gif movie
	this is the version for ZX Spectrum-style screens
*/
void ZxspGifWriter::writeFrame(
	IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline, uint32 cc_start_of_screenfile,
	bool flashphase)
{
	assert(gif_encoder.imageInProgress());
	assert(bits && bits2 && diff && diff2);

	if (update_border || frame_count == 0) bits->setFrame(0, 0, width, height);
	else bits->setFrame(h_border, v_border, screen_width, screen_height);

	drawScreen(
		ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, flashphase); // bits := new screen
	*diff = *bits;

	if (frame_count == 0) {} // first screen
	else					 // subsequent screen
	{
		diff->reduceToDiff(*bits2, global_colormap.transpColor());
		if (diff->isEmpty()) // no change -> increase duration
		{
			frame_count++;
			return;
		}
		else // screen changed -> write old screen to file
		{
			write_diff2_to_file();
		}
	}

	std::swap(bits, bits2);
	std::swap(diff, diff2);
	frame_count = 1;
}


/*	Save the screenshot already rendered in bits[] and bits2[] to file
	this is the version for ZX Spectrum-style screens
*/
void ZxspGifWriter::saveScreenshot(
	cstr path, IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint cc_per_scanline,
	uint32 cc_start_of_screenfile)
{
	assert(!gif_encoder.imageInProgress());
	assert(!bits && !bits2); // else we'd need to fix the bbox

	drawScreen(ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, 0);
	std::swap(bits, bits2);
	drawScreen(ioinfo, ioinfo_count, attr_pixels, cc_per_scanline, cc_start_of_screenfile, 1);
	bits2->reduceToDiff(*bits, global_colormap.transpColor());

	// Calculate Colormaps and total number of colors in image:
	Colormap cmap = global_colormap;
	bits->reduceColors(cmap);
	Colormap cmap2 = global_colormap;
	bits2->reduceColors(cmap2);
	uint total_colors = cmap2.usedColors();
	for (int i = 0; i < cmap.usedColors(); i++) { total_colors += cmap2.findColor(cmap[i]) == Colormap::not_found; }

	// Write to file:
	gif_encoder.openFile(path);
	gif_encoder.writeScreenDescriptor(width, height, total_colors, 0 /*aspect_ratio*/);
	gif_encoder.writeCommentBlock(
		usingstr("created on %s by %s with %s %s\n", datestr(now()), getUser(), APPL_NAME, APPL_VERSION_STR));

	if (bits2->isEmpty()) // no flashing bits
	{
		gif_encoder.writeImage(*bits, cmap);
	}
	else // flashing
	{
		uint delay = (frames_per_flashphase * 100 + frames_per_second / 2) / frames_per_second;
		gif_encoder.writeLoopingAnimationExtension();
		gif_encoder.writeGraphicControlBlock(delay);
		gif_encoder.writeImage(*bits, cmap);
		gif_encoder.writeGraphicControlBlock(delay, cmap2.transpColor());
		gif_encoder.writeImage(*bits2, cmap2);
	}

	gif_encoder.closeFile();

	delete bits;
	bits = nullptr;
	delete bits2;
	bits2 = nullptr;
}

#endif

} // namespace zxsp
