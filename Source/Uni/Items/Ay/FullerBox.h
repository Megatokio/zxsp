// Copyright (c) 2004 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ay.h"
#pragma once


class FullerBox : public Ay
{
public:
	explicit FullerBox(Machine*);

protected:
	// Item interface:
	//virtual void powerOn(/*t=0*/ int32 cc) override;
	//virtual void reset(Time, int32 cc) override;
	virtual void input(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//virtual void output(Time, int32 cc, uint16 addr, uint8 byte) override;
	//virtual void audioBufferEnd(Time) override;
	// virtual void	videoFrameEnd	(int32 cc) override;

public:
	// mimik Joystick interface:
	void	   insertJoystick(JoystickID id) volatile { joystick_id = id; }
	JoystickID getJoystickID() const volatile { return joystick_id; }
	cstr	   getIdf() const volatile { return "F"; } // "F" as in "Fuller"

	uint8 peekButtonsFUDLR() const volatile;						 // buttons FUDLR as for Kempston
	uint8 peekButtons() const volatile;								 // buttons F---RLDU
	bool  isConnected() const { return joystick_id != no_joystick; } // meaning changed -> rename?
	int	  getNumPorts() const { return 1; }

protected:
	uint8 getButtonsFUDLR(); // buttons FUDLR as for Kempston

	JoystickID joystick_id = no_joystick;
};
