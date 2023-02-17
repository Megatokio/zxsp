#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Printer.h"


class PrinterAerco : public Printer
{
public:
	explicit PrinterAerco(Machine*);

protected:
	// Item interface:
	// void	powerOn			( /*t=0*/ int32 cc ) override;
	// void	reset			( Time t, int32 cc ) override;
	// void	input			( Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask ) override;
	// void	output			( Time t, int32 cc, uint16 addr, uint8 byte ) override;
	// void	audioBufferEnd	( Time t ) override;
	// void	videoFrameEnd	( int32 cc ) override;
};
