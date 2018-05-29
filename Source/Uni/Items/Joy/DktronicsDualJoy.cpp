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

#include "DktronicsDualJoy.h"
#include "globals.h"


#define	KI_ADDR	"----.----.000-.----"       // Kempston Issue 4
#define	SI_ADDR	"---0.----.----.---0"       // right Sinclair, Port 0xeffe
#define I_ADDR  "----.----.----.----"
#define O_ADDR  NULL

#define k_bits  0x0000
#define k_mask  0x00e0

#define s_bits  0x0000
#define s_mask  0x1001


/*  TODO

    Der Kempston-Port wird wie das Kempston-IF issue 4 behandelt.
    Das stimmt aber vermutlich so nicht. TODO
    Mögliche Abweichungen:
    - Port-Dekodierung: dekodierte Bits
    - Setzen der unbenutzen Bits
*/


DktronicsDualJoy::DktronicsDualJoy(Machine*m)
:
	Joy(m,isa_DktronicsDualJoy,external,O_ADDR,I_ADDR,"dk","K")
{}


//virtual
void DktronicsDualJoy::input( Time, int32 , uint16 addr, uint8& byte, uint8& mask )
{
    if(!(addr&k_mask))  // Kempston Port (left port)
    {
        // kempston issue 4 data bits:  %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 0
        mask = 0xff;                        // all bits set
        byte = machine==front_machine ? joy[0]->getState() : 0x00;
    }
    if(!(addr&s_mask))  // Sinclair Port (right port)
    {
    	uint8 rval;
        if((rval = joy[1]->getState()))		// state =     %000FUDLR active high
        {                                   // Sinclair 1: %000LRDUF active low
            rval = ((rval&0x10)>>4)
                 | ((rval&0x08)>>2)         // Note: Beware of crap operator precedence!
                 | ((rval&0x04))
                 | ((rval&0x03)<<3);
            mask |= rval;					// only pull down.
            byte &= ~rval;
        }
    }
}









