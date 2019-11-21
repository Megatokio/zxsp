#pragma once
/*	Copyright  (c)	Günter Woigk 2006 - 2019
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

















