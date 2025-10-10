#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "zxsp_types.h"

class Joy : public Item
{
	friend class AyForTc2068;

protected:
	JoystickID joystick_id[3] = {no_joystick, no_joystick, no_joystick};
	cstr	   joystick_idf[3]; // short identifier, e.g. "K" for Kempston
	uint	   num_ports;

public:
	void insertJoystick(uint port, JoystickID id) volatile { this->joystick_id[port] = id; }

	JoystickID getJoystickID(uint port) const volatile { return joystick_id[port]; }
	cstr	   getIdf(uint port) const volatile { return joystick_idf[port]; }

	uint8 peekButtonsFUDLR(uint port) const volatile;								// buttons FUDLR as for Kempston
	bool  isConnected(uint port) const { return joystick_id[port] != no_joystick; } // meaning changed -> rename?
	uint  getNumPorts() const { return num_ports; }

protected:
	Joy(Machine*, isa_id, Internal, cstr o_addr, cstr i_addr, cstr idf1, cstr idf2 = nullptr, cstr idf3 = nullptr);
	~Joy() override = default;

	uint8 getButtonsFUDLR(uint port); // buttons FUDLR as for Kempston

	// Item interface
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
};
