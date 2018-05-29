/*	Copyright  (c)	Günter Woigk 2008 - 2018
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

#ifndef ULAMONO_H
#define ULAMONO_H

#include "Ula.h"


class UlaMono : public Ula
{
protected:
	uint8*	frame_data;				// buffer for decoded monochrome video signal
	uint8*	frame_data2;			// buffer for decoded monochrome video signal

	int		frame_w;				// frame width [bytes] == address offset per scan line
	int		screen_w;				// screen width [bytes]
	int		screen_x0;				// hor. screen offset inside frame scan lines [bytes]

	//int	frame_height;			// frame height [scan line]
	//int	lines_in_screen;		// screen height [scan lines]
	//int	lines_before_screen;	// vert. screen offset inside frame [scan lines]

	//ScreenMono* screen;

protected:
	UlaMono(Machine*, isa_id, cstr oaddr, cstr iaddr);
	~UlaMono();

public:

// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	//void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	static const int
		max_bytes_per_row	= 60,
		max_rows_per_frame	= 320,
		max_bytes_per_frame	= max_bytes_per_row * max_rows_per_frame,
		frame_data_alloc	= max_bytes_per_frame + max_bytes_per_row;

	uint8*	newPixelArray()	{ return new uint8[frame_data_alloc]; }

	void	markVideoRam() override;
VIR void	crtcRead(int32, uint);
VIR int32	getCurrentFramebufferIndex() = 0;
VIR int32	framebufferIndexForCycle(int32 cc) = 0;
};


#endif









