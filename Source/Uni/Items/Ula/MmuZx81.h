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

#ifndef MMUZX81_H
#define MMUZX81_H

#include "MmuZx80.h"


class MmuZx81 : public MmuZx80
{
public:
	explicit MmuZx81(Machine* m)	: MmuZx80(m,isa_MmuZx81) {}

protected:
	MmuZx81(Machine* m, isa_id id)	: MmuZx80(m,id){}

// Item interface:
	void	powerOn			( /*t=0*/ int32 cc ) override;
	//void	reset			( Time t, int32 cc ) override;
	//void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	//void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	//void	audioBufferEnd	( Time t ) override;
	//void	videoFrameEnd	( int32 cc ) override;
	//void	saveToFile		( FD& ) const throws override;
	//void	loadFromFile	( FD& ) throws override;

// Mmu interface:
	//bool	hasPort7ffd	() volatile const noexcept override	{ return no; }
	//bool	hasPort1ffd	() volatile const noexcept override	{ return no; }
	//bool	hasPortF4	() volatile const noexcept override	{ return no; }
	void	romCS		(bool disable) override;  // called from read-side item ----> daisy chain
	//void	ramCS		(bool disable) override;  // called from read-side item ----> daisy chain
						// currently not used. Ram extensions simply add to machine.ram.
};


#endif









