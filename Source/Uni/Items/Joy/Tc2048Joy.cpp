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

#include "Tc2048Joy.h"
#include "Machine.h"



/*

http://8bit.yarek.pl/computer/zx.tc2048/index-de.html:

	In Timex Computer 2048 ULA is seen only in #FE port.
	Other ports used are #F4 (memory switching, not used in TC2048),
	#FF (screen mode, memory switching and interrupts disable) and Kempston Joystick interface.
	And the Kempston Joystick interface is the problem here, as is decoded only with line A5=0.
	So you have to read joystick in ports 0..31, 64..95, 128..160 and 192..224.
	This gives conflict with many external devices and there's no way to disable the input port without internal changes.
	(There are also #F5 and #F6 ports decoded for AY-3-8912, but since no device is connected to decoding lines, they don't make any problem.)

	address:	%xxxx.xxxx.xx0x.xxxx
	data byte:	%000FUDLR active high

	state of unused bits is not clear.

	there is a drawing by yarek.com for a TC2048 kempston joystick port,
	but this probably only addresses the problem of inverted inputs (shorting to +5V)
	which does not work with auto fire joysticks.
*/


#define o_addr	NULL
#define	i_addr	"----.----.--0-.----"



Tc2048Joy::Tc2048Joy ( Machine* m )
:	KempstonJoy(m,isa_Tc2048Joy,internal)
{
	xlogIn("new Tc2048Joy");
}



//void Tc2048Joy::input ( Time/*t*/, int32 /*cc*/, uint16 /*addr*/, uint8& byte, uint8& mask )
//{
//	// %000FUDLR  =>  all bits set:  D0-D4 = 0/1 from js;  D5-D7 = 1
//	mask = 0xff;
//	byte = machine==frontMachine ? joystick()->getState() : 0;
//}









