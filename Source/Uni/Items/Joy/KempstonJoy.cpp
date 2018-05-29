/*	Copyright  (c)	Günter Woigk 2006 - 2018
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

#include "KempstonJoy.h"
#include "Machine.h"



/*														kio 2007-04-29
	kempston:	issue 4, 1989

	address:	%xxxx.xxxx.000x.xxxx		&  /RD=0  &  /IORQ=0  &  /M1=1
				only A5, A6, A7 are decoded and compared to %000 via 74LS138

	data byte:	%000FUDLR active high		via 74LS366 hex inverting driver
				D5 = 0  via 6th driver
				D6 = 0  via diode
				D7 = 0  via diode
*/


//    WoS:
//    #define P_KEMPSTON                      0x1f    /* Port address */
//    #define B_KEMPSTON                      0x20    /* ---- ---- --0- ---- */


//#define  o_addr	NULL
//#define  i_addr	"----.----.000-.----"       // Issue 4



KempstonJoy::KempstonJoy ( Machine* m, isa_id id, Internal i, cstr i_addr )
:	Joy(m,id,i,NULL,i_addr,"K")
{
	xlogIn("new KempstonJoy");
}


KempstonJoy::~KempstonJoy()
{
	xlogIn("~KempstonJoy");
}


void KempstonJoy::input ( Time/*t*/, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
{
	// kempston issue 4 data bits:  %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 0
	mask = 0xff;
	byte &= machine==front_machine ? joy[0]->getState() : 0x00;
}









