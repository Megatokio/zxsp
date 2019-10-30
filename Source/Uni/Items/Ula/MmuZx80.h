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

#ifndef MMUZX80_H
#define MMUZX80_H

#import "Mmu.h"



class MmuZx80 : public Mmu
{
protected:
			MmuZx80         (Machine*m, isa_id id)	: Mmu(m,id,0,0){}

public:
			MmuZx80			(Machine*m)				: Mmu(m,isa_MmuZx80,0,0){}
virtual		~MmuZx80		()						{}

// Item interface:
VIR void	powerOn			(/*t=0*/ int32 cc);
//VIR void	reset			(Time t, int32 cc);
//VIR void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask);
//VIR void	output			(Time t, int32 cc, uint16 addr, uint8 byte);
//VIR void	audioBufferEnd	(Time t);
//VIR void	videoFrameEnd	(int32 cc);
//VIR void	saveToFile		(FD& fd)  const         noexcept(false) /*file_error,bad_alloc*/;
//VIR void	loadFromFile	(FD& fd)				noexcept(false) /*file_error,bad_alloc*/;
void	mapMem();

//VIR bool	hasPort7ffd		()	volatile const noexcept       { return no; }
//VIR bool	hasPort1ffd		()	volatile const noexcept       { return no; }
//VIR bool	hasPortF4		()	volatile const noexcept       { return no; }


/*	ramCS: currently not used. Ram extensions simply add to machine.ram.
*/
//VIR void	ramCS			(bool);

/*	romCS: this signal was not present on the ZX80
*/
//VIR void	romCS			(bool);
};




#endif









