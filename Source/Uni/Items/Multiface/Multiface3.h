#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface.h"


class Multiface3 final : public Multiface
{
	friend class gui::Multiface3Insp;

	bool  mf_enabled; // camouflage FF
	bool  all_ram;	  // port 1FFD.bit0
	uint8 register4x4[4];

public:
	explicit Multiface3(Machine*);

protected:
	~Multiface3() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	( Time t ) override;
	// void	videoFrameEnd	( int32 cc ) override;
	uint8 handleRomPatch(uint16 pc, uint8 o) override; // returns new opcode
	void  triggerNmi() override;
	// uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	// void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};
