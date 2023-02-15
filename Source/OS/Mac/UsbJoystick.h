#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joystick.h"
#include "Templates/Array.h"
#include "cpp/cppthreads.h"
#include <IOKit/hid/IOHIDLib.h>
#include <QObject>


class UsbJoystick : public Joystick
{
	mutable PLock					  lock; // for dev_if
	mutable IOHIDDeviceInterface122** dev_if;

	IOHIDElementCookie		  x_axis;
	IOHIDElementCookie		  y_axis;
	Array<IOHIDElementCookie> buttons;

	uint8 get_state() const;

public:
	UsbJoystick();
	~UsbJoystick();

	virtual uint8 getState(bool) volatile;
	virtual bool  isConnected() const volatile { return dev_if != 0; }

private:
	bool getCookies();
	void connect(io_object_t);
	void disconnect(); // warning: there is also a "QObject::disconnect()"

	friend void findUsbJoysticks();
};
