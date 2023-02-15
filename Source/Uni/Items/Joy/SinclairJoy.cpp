// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "SinclairJoy.h"
#include "globals.h"


/*
	class SinclairJoy can serve as base class for all joystick interfaces which work like the ZX Spectrum +2 joysticks.
	e.g. ZX Spectrum +2, +3, +2A, and Sinclair Interface II.

	it is assumed that the pressed keys are only driven low by oK drivers
	and that not affected bits are not driven at all.
*/


#define O_ADDR nullptr
#define I_ADDR "----.----.----.---0"


SinclairJoy::SinclairJoy(Machine* m, isa_id id, Internal internal, cstr i_addr) :
	Joy(m, id, internal, O_ADDR, i_addr, "S1", "S2")
{}


/*
   WoS:
	The 'left' Sinclair joystick maps the joystick directions and the fire button to the 1 (left), 2 (right), 3 (down),
   4 (up) and 5 (fire) keys on the ZX Spectrum keyboard, and can thus be read via port 0xf7fe; see the Port 0xfe section
   for full details. For any of the joystick interfaces which map to keys, any game offering the appropriate form of
   joystick control can instead be played with the listed keys. The 'right' Sinclair joystick maps to keys 6 (left), 7
   (right), 8 (down), 9 (up) and 0 (fire) and can therefore be read via port 0xeffe.

	sinclair1:
		data byte:	%000FUDRL active low
		keys:           54321
		address:	0xF7FE
					%----.0---.----.---0

	sinclair2:
		data byte:	%000LRDUF active low
		keys:           67890
		address:	0xEFFE
					%---0.----.----.---0

	Gemischte Abfrage mit addr = 0xE7FE ?               TODO
	Wert der oberen 3 Datenbits noch nicht geklärt.		TODO
*/

// joystick.state = %000FUDLR
//		button1_mask		= 0x10,
//		button_up_mask		= 0x08,
//		button_down_mask	= 0x04,
//		button_left_mask	= 0x02,
//		button_right_mask	= 0x01


void SinclairJoy::input(Time, int32, uint16 addr, uint8& byte, uint8& mask)
{
	uint8 jbyte;

	if (machine == front_machine)
	{
		if ((~addr & 0x0800) && (jbyte = joy[0]->getState())) // state =     %000FUDLR active high
		{													  // Sinclair 1: %000FUDRL active low
			jbyte = calcS1ForJoy(jbyte);					  //                 54321
			byte &= jbyte;
			mask |= ~jbyte; // oK: pull down only.
		}

		if ((~addr & 0x1000) && (jbyte = joy[1]->getState())) // state =     %000FUDLR active high
		{													  // Sinclair 2: %000LRDUF active low
			jbyte = calcS2ForJoy(jbyte);					  //                 67890
			byte &= jbyte;
			mask |= ~jbyte; // oK: pull down only.
		}
	}
}
