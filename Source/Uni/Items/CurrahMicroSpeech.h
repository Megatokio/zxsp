#pragma once
// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Memory.h"
class SP0256;


class CurrahMicroSpeech : public Item
{
	SP0256*	  sp0256;
	MemoryPtr rom;

	bool enable_state; // rom paged in and i/o ports enabled
	void toggle_enable_state();

	// for display in Inspector:
	uint8 pitch; // 0x00 or 0x40
	void  add_history(uint8);

	Frequency clock, current_clock;

public:
	uint8 history[16];
	uint  lastrp, lastwp;
	uint  pause; // after history[wp]


public:
	explicit CurrahMicroSpeech(Machine*);

	void setHifi(bool); // main thread only
	bool isHifi() const volatile;

protected:
	~CurrahMicroSpeech() override;

	// Item interface:
	void  powerOn(/*t=0*/ int32 cc) override;
	void  reset(Time t, int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	uint8 handleRomPatch(uint16, uint8) override; // returns new opcode
	void  audioBufferEnd(Time t) override;
	// void	videoFrameEnd(int32 cc) override;

	uint8 readMemory(Time t, int32 cc, uint16 addr, uint8 byte) override;  // memory mapped i/o
	void  writeMemory(Time t, int32 cc, uint16 addr, uint8 byte) override; // memory mapped i/o
};
