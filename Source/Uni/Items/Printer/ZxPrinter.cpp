/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#include "ZxPrinter.h"


//    Wos:
//    Peripheral: ZX Printer.
//    Port: ---- ---- ---- -0--
//
//  The ZX Printer is addressed in the same way as the Alphacom 32 and Timex TS2040, with the following notes:
//    D0 and D7 are both latched so that they remain high until the computer writes something to the printer. So even if you don't make use of the information you've read in, you should output an instruction (with appropriate data) to reset the latches until the next signal. These bits may be in either state on switch on, and aren't affected by the feed button.
//    The paper detect signal is also used internally by the printer to make sure that the styli stop off the paper. Note that if power is applied to the stylus, the paper signal will go high even if the printer is between scans, so the stylus must be turned off before attempting to detect the edge of the paper.


#define o_addr  "---- ---- ---- -0--"
#define i_addr NULL

ZxPrinter::ZxPrinter(Machine*m)
: Printer(m,isa_ZxPrinter,external,o_addr,i_addr)
{
}
