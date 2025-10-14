// Copyright (c) 2006 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Templates/Array.h"
#include "cpp/cppthreads.h"
#include <IOKit/hid/IOHIDLib.h>
#ifdef NDEBUG
  #undef assert // <IOHIDLib.h> includes <CoreFoundation.h> which includes <assert.h> which redefines assert:
  #define assert(COND) (!debug || (COND) ? (void)0 : kio::panic("assert: %s:%i", filenamefrompath(__FILE__), __LINE__))
#endif

class UsbJoystick;

extern uint					num_usb_joysticks;
extern volatile UsbJoystick usb_joysticks[MAX_USB_JOYSTICKS];

extern void findUsbJoysticks();


class UsbJoystick
{
	friend void findUsbJoysticks();

	PLock					  mutex;
	IOHIDDeviceInterface122** dev_if = nullptr;

	IOHIDElementCookie		  x_axis;
	IOHIDElementCookie		  y_axis;
	Array<IOHIDElementCookie> buttons;

public:
	UsbJoystick() noexcept = default;
	~UsbJoystick() { disconnect(); }
	void lock() volatile { mutex.lock(); }
	void unlock() volatile { mutex.unlock(); }

	uint8 getState();
	bool  isConnected() const volatile { return dev_if != nullptr; }

private:
	bool get_cookies();
	bool connect(io_object_t);
	void disconnect();
};
