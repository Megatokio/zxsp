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
	JoystickID joystick_id[3] = {no_joystick};
	cstr	   joystick_idf[3]; // short identifier, e.g. "K" for Kempston
	uint	   num_ports;

public:
	void insertJoystick(uint idx, JoystickID id) { this->joystick_id[idx] = id; }

	JoystickID getJoystickID(uint i) const volatile { return joystick_id[i]; }
	cstr	   getIdf(uint i) const volatile { return joystick_idf[i]; }

	uint8 getButtonsFUDLR(uint i) const;									  // buttons FUDLR as for Kempston
	bool  isConnected(uint i) const { return joystick_id[i] != no_joystick; } // meaning changed -> rename?
	uint  getNumPorts() const { return num_ports; }

protected:
	Joy(Machine*, isa_id, Internal, cstr o_addr, cstr i_addr, cstr idf1, cstr idf2 = nullptr, cstr idf3 = nullptr);
	~Joy() override = default;

	// Item interface
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
};
