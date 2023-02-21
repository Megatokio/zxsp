#pragma once
// Copyright (c) 2006 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Joystick.h" // physical joysticks


class Joy : public Item
{
	friend class AyForTc2068;

protected:
	Joystick*			  joy[3];
	cstr				  idf[3];
	gui::OverlayJoystick* overlays[3];
	uint				  num_ports;

public:
	void insertJoystick(int i, int id);
	void insertJoystick(int id) { insertJoystick(0, id); }

	JoystickID				 getJoystickID(int i = 0) const volatile { return indexof(joy[i]); }
	const Joystick*			 joystick(int i = 0) const { return joy[i]; }
	const volatile Joystick* joystick(int i = 0) const volatile { return joy[i]; }

	uint8 getStateForInspector(int i = 0) const volatile { return joy[i]->getState(no); }
	bool  isConnected(int i = 0) const volatile { return joy[i]->isConnected(); }
	uint  getNumPorts() const volatile { return num_ports; }

protected:
	Joy(Machine*, isa_id, Internal, cstr o_addr, cstr i_addr, cstr idf1, cstr idf2 = nullptr, cstr idf3 = nullptr);
	~Joy() override;

	// Item interface
	// void	powerOn			(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	// uint8	handleRomPatch	(uint16 pc, uint8 o) override;	// returns new opcode
	// void	triggerNmi		() override;
	// uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	// void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};
