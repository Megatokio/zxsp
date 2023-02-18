#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	this file implements a virtual base class for real joysticks.
	subclasses are:
						KbdJoystick			joystick emulation on keyboard
						UsbJoystick			usb joysticks.

	It provides a list of 4 joysticks, intended max. 3 USB joysticks and 1 keyboard joystick.
	The list is populated with a call to FindJoysticks() during appl initialization.
	These 4 Joystick instances are never deleted. They are only disconnected when getState() fails.
	To discover new USB joysticks call findUsbJoysticks().
*/

#include "IsaObject.h"
#include "kio/kio.h"
#include "zxsp_types.h"


enum JoystickID { usb_joystick0 = 0, usb_joystick1 = 1, usb_yoystick2 = 2, kbd_joystick = 3, no_joystick = 4 };

const int num_usb = 3;
const int max_joy = 5;

extern Joystick* joysticks[max_joy];
#define usbJoystick(N)	 reinterpret_cast<UsbJoystick*>(joysticks[N])
#define keyboardJoystick reinterpret_cast<KbdJoystick*>(joysticks[kbd_joystick])
#define noJoystick		 (joysticks[no_joystick])

extern void		  findUsbJoysticks();
inline JoystickID indexof(Joystick* p)
{
	int i = 0;
	while (i < no_joystick && p != joysticks[i]) i++;
	return JoystickID(i);
}
inline JoystickID indexof(Joystick** p) { return JoystickID(p - joysticks); }


class Joystick : public IsaObject
{
protected:
	Time  last_time; // for activity monitoring
	uint8 state;	 // %000FUDLR

	explicit Joystick(isa_id id) : IsaObject(nullptr, id, isa_Joystick), last_time(0), state(0) {}
	~Joystick() override {}

public:
	virtual uint8 getState(bool update_last_time = yes) volatile = 0;
	virtual bool  isConnected() const volatile { return yes; }
	uint		  isActive() const volatile { return system_time < last_time + 2.0; }

	enum Button //	= %000FUDLR
	{
		button1_mask	  = 0x10,
		button_up_mask	  = 0x08,
		button_down_mask  = 0x04,
		button_left_mask  = 0x02,
		button_right_mask = 0x01
	};
};


// _______________________________________________________________________
//


class NoJoystick : public Joystick
{
public:
	NoJoystick() : Joystick(isa_Joystick) {}
	uint8 getState(bool) volatile { return 0x00; } // no keys pressed
};


// _______________________________________________________________________
//


class KbdJoystick : public Joystick
{
public:
	KbdJoystick() : Joystick(isa_KbdJoystick) {}

	uint8 getState(bool f) volatile
	{
		if (f) last_time = system_time;
		return state;
	}

	void keyDown(uint mask) volatile { state |= mask; } // %000FUDLR
	void keyUp(uint mask) volatile { state &= ~mask; }	// %000FUDLR
	void allKeysUp() volatile { state = 0; }
};
