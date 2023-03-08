#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


/*  The Timex TS2068 and TC2068 joysticks are attached to port A of the AY-3-8912 sound chip.
	Reading port A of the sound chip is handled in the AyForTc2068 class.

	The bits read from port A have the following meaning:

		bit 0:		0 = up
		bit 1:		0 = down
		bit 2:		0 = left
		bit 3:		0 = right
		bit 4:		0 = fire		i believe this was WOS and seems to be wrong
		bits 5-7:	111				SAMS says bit 7 is fire which seems to be true
*/


class Tc2068Joy : public Joy
{
public:
	Tc2068Joy(Machine* m, isa_id id = isa_Tc2068Joy) : Joy(m, id, internal, nullptr, nullptr, "J1", "J2") {}

	// Item interface:
	void input(Time, int32, uint16, uint8&, uint8&) override {}

	// helper: convert %000FUDLR active high -> %F111RLDU active low
	static inline uint8 calcButtonsFromFUDLR(uint8 joy)
	{
		return ~(((joy << 3) & 0x80) | ((joy >> 3) & 1) | ((joy >> 1) & 2) | ((joy << 1) & 4) | ((joy << 3) & 8));
	}

	uint8 getButtonsF111RLDU(uint i) const { return calcButtonsFromFUDLR(getButtonsFUDLR(i)); }
};


class Ts2068Joy : public Tc2068Joy
{
public:
	explicit Ts2068Joy(Machine* m) : Tc2068Joy(m, isa_Ts2068Joy) {}
};


class U2086Joy : public Tc2068Joy
{
public:
	explicit U2086Joy(Machine* m) : Tc2068Joy(m, isa_U2086Joy) {}
};
