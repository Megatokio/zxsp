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


#include "FdcJLO.h"


//    WoS:

//    Peripheral: JLO (Status/Command).
//    Port: ---- ---- 1000 1111

//    Peripheral: JLO (Track).
//    Port: ---- ---- 1001 1111

//    Peripheral: JLO (Sector).
//    Port: ---- ---- 1010 1111

//    Peripheral: JLO (Data).
//    Port: ---- ---- 1011 1111

//    Peripheral: JLO (Select).
//    Port: ---- ---- 1011 0111

/*	1.  Port # 1000 1111, RW, read status/write command reg
	2.  Port # 1001 1111, RW, read/write track reg
	3.  Port # 1010 1111, RW, read/write sector reg
	4.  Port # 1011 1111, RW, read/write data reg
	5.  Port # 1011 0111,  W, drive/density/side select
*/

static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

FdcJLO::FdcJLO(Machine*m)
:    Fdc(m,isa_FdcJLO,external,o_addr,i_addr)
{
}
