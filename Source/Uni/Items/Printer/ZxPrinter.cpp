/*	Copyright  (c)	Günter Woigk 2009 - 2018
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
