/*	Copyright  (c)	Günter Woigk 1995 - 2018
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

#ifndef CRTC_H
#define CRTC_H

#include "kio/kio.h"
#include "Item.h"


class Crtc : public Item
{
protected:
	const ZxInfo* info;				// machine info
	Screen* screen;
	CoreByte* video_ram;			// current video ram

	static const int
			bytes_per_octet = 2,	// number of bytes stored per pixel octet for the renderer
			cc_per_byte		= 4,	// ula cycles per pixel block  ((2 bytes == 8 pixel))
			lines_in_screen = 192;

	int		lines_before_screen;
	int		lines_after_screen;
	int		lines_per_frame;
	int		cc_per_line;

	uint8	border_color;			// current border color
	bool	is60hz;

public:
	uint8	getBorderColor()					{ return border_color; }
VIR void	setBorderColor(uint8)				{}
	CoreByte* getVideoRam()						{ return video_ram; }

	bool	is60Hz() volatile const				{ return is60hz;  }
	bool	is50Hz() volatile const				{ return !is60hz; }

	int		getLinesBeforeScreen() volatile const{ return lines_before_screen; }	// nominal
	int		getLinesInScreen() volatile const	{ return lines_in_screen; }			// nominal
	int		getLinesAfterScreen() volatile const{ return lines_after_screen; }		// nominal
	int		getLinesPerFrame() volatile const	{ return lines_per_frame; }
	int		getCcPerByte() volatile const		{ return cc_per_byte; }				// const
	int		getCcPerLine() volatile const		{ return cc_per_line; }				// nominal
	int		getBytesPerLine() volatile const	{ return cc_per_line/cc_per_byte; }	// nominal
VIR	int32	getCcPerFrame() volatile const		{ return lines_per_frame * cc_per_line; }

	void	attachToScreen(Screen*);
VIR	void	drawVideoBeamIndicator(int32 cc)	= 0;
VIR int32	doFrameFlyback(int32 cc)			= 0;
VIR int32	cpuCycleOfNextCrtRead()				= 0;
VIR int32	updateScreenUpToCycle(int32 cc)		= 0;
VIR	void	markVideoRam()						= 0;
VIR	void	set60Hz(bool f=1)					{ is60hz=f; }
	void	set50Hz()							{ set60Hz(0); }

protected:
	Crtc(Machine*, isa_id, isa_id grp, Internal, cstr o_addr, cstr i_addr);

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;
};


#endif











































