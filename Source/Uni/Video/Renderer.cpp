// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Renderer.h"
#include "graphics/gif/Colormap.h"
#include "unix/os_utilities.h"
#include "globals.h"



Renderer::Renderer(QObject* p, isa_id id, uint screen_width, uint screen_height, uint h_border, uint v_border, bool color)
:
	IsaObject(p,id,isa_Renderer),
	screen_width(screen_width),
	screen_height(screen_height),
	h_border(h_border),
	v_border(v_border),
	width(screen_width+2*h_border),
	height(screen_height+2*v_border),
	mono_octets(new uint8[color?width*height*sizeof(RgbaColor):width*(height+1)/8])
{}




// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================


GifWriter::GifWriter( QObject* p, isa_id id, cColormap& colormap, uint screen_width, uint screen_height,
					  uint h_border, uint v_border, bool update_border, uint frames_per_second )
:
	IsaObject(p,id,isa_GifWriter/*grp*/),
	screen_width(screen_width),
	screen_height(screen_height),
	h_border(h_border),
	v_border(v_border),
	width(screen_width+2*h_border),
	height(screen_height+2*v_border),
	frame_count(0),
	bits(nullptr),
	diff(nullptr),
	bits2(nullptr),
	diff2(nullptr),
	update_border(update_border),
	frames_per_second(frames_per_second),
	frames_per_flashphase(16),
	global_colormap(colormap)
{}


// helper: write diff2[] to file:
void GifWriter::write_diff2_to_file() throws
{
	Colormap cmap = global_colormap; diff2->reduceColors(cmap);
	uint delay = (frame_count*100+frames_per_second/2)/frames_per_second;
	gif_encoder.writeGraphicControlBlock(delay, cmap.transpColor());
	gif_encoder.writeImage(*diff2, cmap);
}


void GifWriter::startRecording( cstr path ) throws
{
	assert(!gif_encoder.imageInProgress());
	assert(!bits&&!bits2&&!diff&&!diff2);

	gif_encoder.openFile(path);
	gif_encoder.writeScreenDescriptor(width, height, 16);
	gif_encoder.writeCommentBlock( usingstr("created on %s by %s with %s %s\n",
								   datestr(now()), getUser(), APPL_NAME, APPL_VERSION_STR) );
	gif_encoder.writeLoopingAnimationExtension();

	frame_count = 0;	// indicate first frame
	bits  = new Pixelmap(width,height);
	diff  = new Pixelmap(width,height);
	bits2 = new Pixelmap(width,height);
	diff2 = new Pixelmap(width,height);
}


void GifWriter::stopRecording() throws
{
	assert(gif_encoder.imageInProgress());

	if(frame_count) write_diff2_to_file();
	gif_encoder.closeFile();

	delete bits;  bits=nullptr;
	delete diff;  diff=nullptr;
	delete bits2; bits2=nullptr;
	delete diff2; diff2=nullptr;
}











