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

#include "unix/FD.h"
#include "Joy.h"
#include "Machine/Machine.h"


Joy::Joy (Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr , cstr idf1, cstr idf2, cstr idf3)
:
	Item(m,id,isa_Joy,internal,o_addr,i_addr),
	joy{NULL,NULL,NULL},
	idf{idf1,idf2,idf3},
	overlays{NULL,NULL,NULL},
	num_ports(idf3?3:idf2?2:1)
{
	xlogIn("new Joy");

// kbd_joystick nur explizit einstecken, wg. "some keys do not react" desaster!
	insertJoystick(0,0);		// usb joystick 0 in port 0
	if(idf2) insertJoystick(1,1);		// usb joystick 1 in port 1
	if(idf3) insertJoystick(2,2);		// usb joystick 2 in port 2
}


Joy::~Joy()
{
	xlogIn("~Joy");
	for(uint i=0;i<NELEM(overlays);i++) { machine->removeOverlay(overlays[i]); }
}


void Joy::insertJoystick( int i, int id )
{
	if(joy[i] == joysticks[id]) return;

	if(overlays[i]) { machine->removeOverlay(overlays[i]); overlays[i]=NULL; }
	joy[i] = joysticks[id];
	if(id!=no_joystick) overlays[i] = machine->addOverlay(joy[i],idf[i],i&1?Overlay::TopLeft:Overlay::TopRight);
}


void Joy::saveToFile( FD& fd ) const noexcept(false) /*file_error,bad_alloc*/
{
	Item::saveToFile(fd);
	switch(getNumPorts())
	{
		case 3:	fd.write_char(getJoystickID(2));
		case 2:	fd.write_char(getJoystickID(1));
		default:fd.write_char(getJoystickID(0));
	}
}

void Joy::loadFromFile( FD& fd ) noexcept(false) /*file_error,bad_alloc*/
{
	Item::loadFromFile(fd);
	switch(getNumPorts())
	{
	case 3:	insertJoystick(2,fd.read_char());
	case 2:	insertJoystick(1,fd.read_char());
	default:insertJoystick(0,fd.read_char());
	}
}








