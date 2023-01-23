#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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














