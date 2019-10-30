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

#ifndef USBJOYSTICK_H
#define	USBJOYSTICK_H

#include <IOKit/hid/IOHIDLib.h>
#include <QObject>
#include "cpp/cppthreads.h"
#include "Joystick.h"
#include "Templates/Array.h"


class UsbJoystick : public Joystick
{
	mutable PLock      lock;		// for dev_if
	mutable IOHIDDeviceInterface122** dev_if;

	IOHIDElementCookie			x_axis;
	IOHIDElementCookie			y_axis;
	Array<IOHIDElementCookie>	buttons;

	uint8		get_state		()	const;

public:
				UsbJoystick		();
				~UsbJoystick	();

virtual	uint8	getState		(bool)	volatile;
virtual bool	isConnected		()		volatile const 		{ return dev_if!=0; }

private:
	bool		getCookies();
	void		connect(io_object_t);
	void		disconnect();						// warning: there is also a "QObject::disconnect()"

friend void		findUsbJoysticks();
};






#endif










