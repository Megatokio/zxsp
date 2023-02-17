#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Items/Joy/Joy.h"
#include "Memory.h"
#include "Multiface.h"
#include "Templates/Array.h"
class OverlayJoystick;


class Multiface1 : public Multiface
{
	friend class Multiface1Insp;

	Joystick*		 joystick;
	OverlayJoystick* overlay;
	bool			 joystick_enabled;

public:
	explicit Multiface1(Machine*);
	virtual ~Multiface1();

	void	   insertJoystick(int id) volatile;
	JoystickID getJoystickID() const volatile { return indexof(joystick); }
	void	   enableJoystick(bool f) volatile { joystick_enabled = f; }

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	uint8 handleRomPatch(uint16 pc, uint8 o) override; // returns new opcode
	void  triggerNmi() override;
	// uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	// void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};
