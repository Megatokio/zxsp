// Copyright (c) 2004 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "FullerBox.h"
#include "Machine.h"

//    $xx7F   %0111.1111      Fuller joystick i/-
//    $xx3F   %0011.1111      Fuller AY reg. select/read i/o
//    $xx5F   %0101.1111      Fuller AY write -/o
//
//    Joystick interface:
//		Result is: F---RLDU, with active bits low.
//		Promised to work with every Imagine software

/* implementation note:
   class FullerBox is based on class Ay:
   - we don't need to intercept output(), and Ay::output() can use the address for sel/data discrimination.
   - we must intercept input() for the joystick, but can forward to Ay::input() if needed without problems,
	 because Ay::input() does not need to (and does not) check the address.
*/

// to be verified:
static cstr sel_ay = "----.----.001-.----"; // $3F select
static cstr wr_ay  = "----.----.010-.----"; // $5F write
static cstr wr_ora = "----.----.100-.----"; // $9F Orator?
static cstr rd_ay  = "----.----.001-.----"; // $3F read
static cstr rd_joy = "----.----.011-.----"; // $7F read joystick

static const Frequency	   ay_freq = 1700000;  // guessed
static const Ay::StereoMix ay_mix  = Ay::mono; // there's no indication that it could have had stereo


FullerBox::FullerBox(Machine* m) : //
	Ay(m, isa_FullerBox, external, sel_ay, wr_ay, And(rd_ay, rd_joy), ay_freq, ay_mix)
{
	if (num_usb_joysticks >= 1) insertJoystick(usb_joystick0);
}

uint8 FullerBox::getButtonsFUDLR()
{
	// return buttons FUDLR as for Kempston
	return machine->getJoystickButtons(joystick_id);
}

uint8 FullerBox::peekButtonsFUDLR() const volatile // for joystick overlay
{
	// return buttons FUDLR as for Kempston
	return machine->peekJoystickButtons(joystick_id);
}

static uint8 buttons_for_buttons(uint b)
{
	// in:  0b---FUDLR active-high as for Kempston
	// out: 0bF---RLDU active-low as for Fuller

	b = ((b & 0x11) << 3) | ((b & 0x02) << 1) | ((b & 0x04) >> 1) | ((b & 0x08) >> 3);
	return uint8(~b);
}

uint8 FullerBox::peekButtons() const volatile // for inspector
{
	// buttons F---RLDU
	return buttons_for_buttons(machine->peekJoystickButtons(joystick_id));
}

void FullerBox::input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	if (addr & 0x40) // %0111.1111 = read joystick
	{
		byte &= buttons_for_buttons(machine->getJoystickButtons(joystick_id));
		mask |= 0xff; // to be verified
	}
	else // %0011.1111 = read Ay register
	{
		Ay::input(t, cc, addr, byte, mask);
	}
}

//void FullerBox::powerOn(/*t=0*/ int32 cc) { Ay::powerOn(cc); }
//void FullerBox::reset(Time t, int32 cc) { Ay::reset(t, cc); }
//void FullerBox::output(Time t, int32 cc, uint16 addr, uint8 byte) { Ay::output(t, cc, addr, byte); }
//void FullerBox::audioBufferEnd(Time t) { Ay::audioBufferEnd(t); }


/*


































*/
