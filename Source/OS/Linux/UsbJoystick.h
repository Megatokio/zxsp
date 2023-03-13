// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "cpp/cppthreads.h"
#include "kio/kio.h"
class UsbJoystick;


extern uint					num_usb_joysticks;
extern volatile UsbJoystick usb_joysticks[MAX_USB_JOYSTICKS];

extern void findUsbJoysticks();


class UsbJoystick
{
	friend void findUsbJoysticks();

	PLock mutex;

public:
	UsbJoystick() noexcept = default;
	~UsbJoystick()		   = default;
	void lock() volatile { mutex.lock(); }
	void unlock() volatile { mutex.unlock(); }

	uint8 getState() { return 0x00; }
	bool  isConnected() const volatile { return false; }
};
