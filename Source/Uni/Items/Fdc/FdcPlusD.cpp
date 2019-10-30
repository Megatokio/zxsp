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


#include "FdcPlusD.h"

/*	Plus D Interface
	up to 2 Disk drives
	1 Centronics port
*/

//    Wos:
//    #define P_PLUSD_CMD                     0xe3    /* Command */
//    #define P_PLUSD_STATE                   0xe3    /* State */
//    #define P_PLUSD_PAGE                    0xe7    /* Memory paging */
//    #define P_PLUSD_TRACK                   0xeb    /* Track */
//    #define P_PLUSD_SYSTEM                  0xef    /* System register */
//    #define P_PLUSD_SECTOR                  0xf3    /* Sector */
//    #define P_PLUSD_PRINTER                 0xf7    /* Printer data/ready */
//    #define P_PLUSD_DATA                    0xfb    /* Data */



static cstr o_addr = NULL;//TODO
static cstr i_addr = NULL;//TODO

FdcPlusD::FdcPlusD(Machine *m)
:    Fdc(m,isa_FdcPlusD,external,o_addr,i_addr)
{
}
