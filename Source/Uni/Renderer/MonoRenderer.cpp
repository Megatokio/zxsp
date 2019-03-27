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

#include "Templates/Array.h"
#include "MonoRenderer.h"
#include <QImage>
#include <QPainter>
#include "gif/Pixelmap.h"
#include "gif/Colormap.h"
#include "gif/GifEncoder.h"
#include "globals.h"
#include "cpp/cppthreads.h"
#include "unix/os_utilities.h"






/*	rendere Ausgaben der B&W Ula in mono_octets[].
*/
void MonoRenderer::drawScreen( uint8* new_pixels, uint q_screen_width, uint q_screen_height,
                               uint q_width, uint q_height, uint q_h_border, uint q_v_border, uint32 cc_vbi )
{
//	assert(!(q_screen_w&7));
	assert(!(q_width&7));
//	assert(!(q_screen_x0&7));
//	assert(q_screen_w<q_frame_w);
//	assert(q_screen_h<q_frame_h);
//	assert(q_frame_w>=screen_width);
//	assert(q_frame_h>=screen_height);

	if(cc_vbi) TODO();

// normalize q_h|v_border to 256x192 pixel screen:
	q_h_border += (screen_width - q_screen_width) / 2;
	q_v_border += (screen_height - q_screen_height) / 2;

// source and destination boxes:
	int qx=0, qy=0, qw=q_width, qh=q_height;	// qbox = source
	int zx=0, zy=0, zw=width,   zh=height;		// zbox = dest

// shift boxes to meet at screen_P0:
	int dx = q_h_border - h_border;	if(dx>0) qx+=dx; else zx-=dx;
	int dy = q_v_border - v_border;	if(dy>0) qy+=dy; else zy-=dy;

// limit box width and height:
	qw = min(qw,int(q_width)-qx);
	qh = min(qh,int(q_height)-qy);
	zw = min(zw,width-zx);
	zh = min(zh,height-zy);

// use smaller width and height:
	int w = min(qw,zw) / 8;
	int h = min(qh,zh);

// offset to add to pointers after each row:
	int qo = q_width/8 - w;
	int zo = width/8 - w;

// source, destination and dest_end pointers:
	uint8 const* qp = new_pixels  + qy*q_width/8 + qx/8;
	uint8*		 zp = mono_octets;
	uint8*		 ze;

// paint pixels before frame buffer data with black color:
	for(ze = zp + zy*width/8 + zx/8; zp<ze;) *zp++ = 0;

// copy frame buffer:
	for(ze = zp + h*width/8; zp<ze;)
	{
		// copy one row:
		for(uint8* ze = zp + w;zp<ze;) *zp++ = *qp++;
		// skip reminder of bytes in row
		for(uint8* ze = zp + zo;zp<ze;) *zp++ = 0;
		qp+=qo;
	}

// paint pixels behind frame buffer data with black color:
	for(ze = mono_octets + height*width/8; zp<ze;) *zp++ = 0;

	assert(ze==mono_octets+width*height/8);
}



// ================================================================================
//		Gif file handling
//		save a screenshot or record movie
// ================================================================================

typedef uint8 GifColor;
//const GifColor gifcolor_black  = 0;
//const GifColor gifcolor_white  = 1;
const GifColor transp = 2;

cComp	mono_colors[] = { 0,0,0, 255,255,255, 0,0,0 };
cColormap mono_colormap(mono_colors,2,transp);
cColormap mono_colormap_with_trans(mono_colors,3,transp);


MonoGifWriter::MonoGifWriter(QObject* p, bool update_border, uint frames_per_second)
:
	GifWriter(p,isa_MonoGifWriter,mono_colormap_with_trans,256,192,32,24,update_border,frames_per_second)
{}


void MonoGifWriter::drawScreen( uint8* new_pixels, uint q_screen_width, uint q_screen_height,
								uint q_width, uint q_height, uint q_h_border, uint q_v_border )
{
//	assert(!(q_screen_w&7));
	assert(!(q_width&7));
//	assert(!(q_screen_x0&7));
//	assert(q_screen_w<q_frame_w);
//	assert(q_screen_h<q_frame_h);
//	assert(q_frame_w>=screen_width);
//	assert(q_frame_h>=screen_height);

	if(!bits) bits = new Pixelmap(width,height);

// normalize q_h|v_border to 256x192 pixel screen:
	q_h_border += (screen_width - q_screen_width) / 2;
	q_v_border += (screen_height - q_screen_height) / 2;

// source and destination boxes:
	int qx=0, qy=0, qw=q_width, qh=q_height;	// qbox = source
	int zx=0, zy=0, zw=width,   zh=height;		// zbox = dest

// shift boxes to meet at screen_P0:
	int dx = q_h_border - h_border;	if(dx>0) qx+=dx; else zx-=dx;
	int dy = q_v_border - v_border;	if(dy>0) qy+=dy; else zy-=dy;

// limit box width and height:
	qw = min(qw,int(q_width)-qx);
	qh = min(qh,int(q_height)-qy);
	zw = min(zw,width-zx);
	zh = min(zh,height-zy);

// use smaller width and height:
	int w = min(qw,zw);
	int h = min(qh,zh);

// offset to add to pointers after each row:
	int qo = q_width/8 - w/8;
	int zo = width - w;

// source, destination and dest_end pointers:
	uint8 const* qp = new_pixels  + qy*q_width/8 + qx/8;
	GifColor*	 zp = bits->getData();
	GifColor*	 ze;
	GifColor*	zee = bits->getData()+width*height;

// paint pixels before frame buffer data with black color:
	for(ze = zp + zy*width + zx; zp<ze;) *zp++ = 0;

// copy frame buffer:
	for(ze = min(zee, zp + h*width); zp<ze;)
	{
		// copy one row:
		for(uint8* ze = zp + w;zp<ze;) { uint8 pixels=*qp++; for(uint s=8;s--;) { *zp++ = (pixels>>s)&1; } }
		// skip reminder of bytes in row
		for(uint8* ze = min(zee, zp + zo);zp<ze;) *zp++ = 0;
		qp+=qo;
	}

// paint pixels behind frame buffer data with black color:
	while(zp<zee) *zp++ = 0;

	assert(zp==zee);
}


/*	append frame to a gif movie
	this is the version for b&w screens
*/
void MonoGifWriter::writeFrame( uint8* new_pixels, uint screen_w, uint screen_h, uint frame_h, uint frame_w,
								uint screen_x0, uint screen_y0) throws
{
	assert(gif_encoder.imageInProgress());
	assert(bits&&bits2&&diff&&diff2);

	if(update_border || frame_count==0) bits->setFrame(0,0, width,height);
	else bits->setFrame(h_border,v_border, screen_width,screen_height);

	drawScreen(new_pixels, screen_w, screen_h, frame_h, frame_w, screen_x0, screen_y0);	// bits := new screen
	*diff = *bits;

	if(frame_count == 0) {}		// first screen
	else						// subsequent screen
	{
		diff->reduceToDiff(*bits2, global_colormap.transpColor());
		if(diff->isEmpty())		// no change -> increase duration
		{
			frame_count++;
			return;
		}
		else					// screen changed -> write old screen to file
		{
			write_diff2_to_file();
		}
	}

	std::swap(bits,bits2);
	std::swap(diff,diff2);
	frame_count = 1;
}


void MonoGifWriter::saveScreenshot( cstr path, uint8* new_pixels, uint screen_w, uint screen_h,
									uint frame_h, uint frame_w, uint screen_x0, uint screen_y0 ) throws
{
	assert(!gif_encoder.imageInProgress());
	assert(!bits);	// else we'd need to fix the bbox

	drawScreen(new_pixels, screen_w, screen_h, frame_h, frame_w, screen_x0, screen_y0);

// Write to file:
    gif_encoder.openFile(path);
	gif_encoder.writeScreenDescriptor(width, height, mono_colormap);
	gif_encoder.writeCommentBlock( usingstr("created on %s by %s with %s %s\n",
								   datestr(now()), getUser(), appl_name,appl_version_str) );
	gif_encoder.writeImage(*bits,mono_colormap);
    gif_encoder.closeFile();

	delete bits; bits=NULL;
}








































