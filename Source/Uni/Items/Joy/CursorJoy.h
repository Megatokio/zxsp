#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


// WoS:
// A cursor joystick interfaces maps to keys 5 (left), 6 (down), 7 (up), 8 (right) and 0 (fire).
// Reading a cursor joystick thus requires a combination of bit 4 of port 0xf7fe and bits 0, 2, 3 and 4 of port 0xeffe.
// Common interfaces offering a cursor joystick option included those produced by Protek and AGF.

class CursorJoy : public Joy
{
public:
	explicit CursorJoy(Machine*);

protected:
	~CursorJoy() override = default;
	CursorJoy(Machine*, isa_id);

	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};


class ProtekJoy final : public CursorJoy
{
	// Kio: The PCB contains two 74LS32 quad OR and one 74LS09 quad AND with oK.

public:
	explicit ProtekJoy(Machine* m) : CursorJoy(m, isa_ProtekJoy) {}

protected:
	~ProtekJoy() override = default;
};
