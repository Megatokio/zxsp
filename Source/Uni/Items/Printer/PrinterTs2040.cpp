/*	Copyright  (c)	Günter Woigk 2012 - 2018
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


#include "PrinterTs2040.h"


//  Timex TS2040  /  Alphacom 32 Printer
//  ------------------------------------



//    WoS:
//    Peripheral: Timex TS2040 / Alphacom 32 Printers.
//    Port: ---- ---- 1111 1011

//    The TS2040 is wired as a z80 I/O port, selected by A2 being at low level and A7 being at high level. No other address lines are recognised. To send information to the printer, use: OUT (FB), A - opcode D3 FB, assuming the data is in register A. The data bits have the following meanings:

//        (D2) High level means stop the motor, low means start it.
//        (D7) High level applies power to the print head.

//    All these lines remain in the state they were last at, until new data is sent to the printer. At switch on, or after pressing the feed button, D7 is set low; D2 is left high once feed is finished. The other data lines are not used.

//    To fetch information from the printer, the z80 instruction: IN A, (FB) - opcode DB FB; will put the data into the accumulator. The following bits are used:

//        (D6) Will be read as low if the printer is there, high if it is not, and is used solely to check if the printer is connected.
//        (D0) This is high when the printer is ready for the next bit.
//        (D7) This line is high for the start of a new line.


static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

PrinterTs2040::PrinterTs2040(Machine *m) :
    Printer(m,isa_PrinterTs2040,external,o_addr,i_addr)
{
}
