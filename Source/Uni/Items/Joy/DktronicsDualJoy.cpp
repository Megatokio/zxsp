// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DktronicsDualJoy.h"
#include "SinclairJoy.h"

#define KI_ADDR "----.----.000-.----" // Kempston Issue 4
#define SI_ADDR "---0.----.----.---0" // right Sinclair, Port 0xeffe
#define I_ADDR	"----.----.----.----"
#define O_ADDR	nullptr

#define k_bits 0x0000
#define k_mask 0x00e0

#define s_bits 0x0000
#define s_mask 0x1001


/* The Kempston joystick port is handled like a Kempston interface issue 4.
	
   TODO: check actually decoded address bits
   TODO: check unused data bits are actually 000
*/


DktronicsDualJoy::DktronicsDualJoy(Machine* m) : Joy(m, isa_DktronicsDualJoy, external, O_ADDR, I_ADDR, "dk", "K") {}

void DktronicsDualJoy::input(Time, int32, uint16 addr, uint8& byte, uint8& mask)
{
	if (!(addr & k_mask)) // Kempston Port (left port)
	{
		byte = getButtonsFUDLR(0); // kempston: %000FUDLR
		mask = 0xff;			   // all bits set
	}

	if (!(addr & s_mask)) // Sinclair Port (right port)
	{
		if (uint8 rval = getButtonsFUDLR(1)) // state =     %000FUDLR active high
		{									 // Sinclair 1: %000LRDUF active low
			byte = SinclairJoy::calcS2FromFUDLR(rval);
			mask |= ~byte; // only pull down.
		}
	}
}
