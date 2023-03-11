#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/Joy.h"
#include "Memory.h"
#include "Multiface.h"
#include "Templates/Array.h"


class Multiface1 final : public Multiface
{
	JoystickID joystick_id;
	bool	   joystick_enabled;

public:
	Multiface1(Machine*, bool enable_joystick);

	void	   insertJoystick(JoystickID id) volatile { joystick_id = id; }
	JoystickID getJoystickID() const volatile { return joystick_id; }
	void	   enableJoystick(bool f) volatile { joystick_enabled = f; }
	cstr	   getIdf() const volatile { return "K"; } // Kempston joystick
	uint8	   peekJoystickButtonsFUDLR() const volatile;
	bool	   isJoystickEnabled() const volatile { return joystick_enabled; }

protected:
	~Multiface1() override = default;

	uint8 getJoystickButtonsFUDLR();

	// Item interface:
	void  powerOn(int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	uint8 handleRomPatch(uint16 pc, uint8 o) override; // returns new opcode
	void  triggerNmi() override;
};
