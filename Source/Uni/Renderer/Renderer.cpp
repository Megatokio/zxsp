/*	Copyright  (c)	Günter Woigk 2013 - 2018
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

#include "Renderer.h"
#include "gif/Colormap.h"
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
	bits(NULL),
	diff(NULL),
	bits2(NULL),
	diff2(NULL),
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
								   datestr(now()), getUser(), appl_name, appl_version_str) );
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

	delete bits;  bits=NULL;
	delete diff;  diff=NULL;
	delete bits2; bits2=NULL;
	delete diff2; diff2=NULL;
}











