/*	Copyright  (c)	Günter Woigk 2008 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#ifndef MMUZX80_H
#define MMUZX80_H

#include "Mmu.h"



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









