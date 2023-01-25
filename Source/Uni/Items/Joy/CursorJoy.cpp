// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "CursorJoy.h"
#include "globals.h"


/*
	class CursorJoy can serve as subclass for joysticks which emulate key presses to "6" ... "0"
	e.g. Protek and A.G.F.

	it is assumed that the pressed keys are only driven low by oK drivers
	and that not affected bits are not driven at all.
*/


#define O_ADDR  nullptr
#define I_ADDR  "----.----.----.---0"


// public c'tor:
CursorJoy::CursorJoy(Machine*m)
:
	Joy(m,isa_CursorJoy,external,O_ADDR,I_ADDR,"C")
{}

// protected c'tor:
CursorJoy::CursorJoy(Machine* m, isa_id id)
:
	Joy(m,id,external,O_ADDR,I_ADDR,"C")
{}



// Cursor:
//      left  -> key "5" -> bit 4   port 0xf7fe
//      down  -> key "6" -> bit 4   port 0xeffe
//      up    -> key "7" -> bit 3   port 0xeffe
//      right -> key "8" -> bit 2   port 0xeffe
//      fire  -> key "0" -> bit 0   port 0xeffe

// joystick.state = %000FUDLR
//		button1_mask		= 0x10,
//		button_up_mask		= 0x08,
//		button_down_mask	= 0x04,
//		button_left_mask	= 0x02,
//		button_right_mask	= 0x01



//virtual
void CursorJoy::input( Time, int32 , uint16 addr, uint8& byte, uint8& mask )
{
	uint8 state;
	if( machine==front_machine && (~addr & 0x1800) && (state = joy[0]->getState()) )
	{
		uint8 mybyte = 0;

		if(~addr & 0x0800)  // 0xf7fe
		{
			mybyte |= (state&2)<<3;   // left -> key "5" -> bit 4
		}
		if(~addr & 0x1000)  // 0xeffe
		{
			mybyte |= ((state&1)<<2)    // right
					+ ((state&4)<<2)    // down
					+ ((state&8)<<0)    // up
					+ ((state&16)>>4);  // fire
		}
		byte &= ~mybyte;    // oK => only 0-bits make it to the bus
		mask |=  mybyte;
	}
}














