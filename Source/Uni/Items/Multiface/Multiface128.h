#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface.h"


class Multiface128 final : public Multiface
{
	bool  mf_enabled;
	uint8 videopage; // video page bit in port 7FFD

public:
	explicit Multiface128(Machine*);

	bool isEnabled() const volatile { return mf_enabled; }

protected:
	~Multiface128() override = default;

	// Item interface:
	void  powerOn(/*t=0*/ int32 cc) override;
	void  reset(Time t, int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	uint8 handleRomPatch(uint16 pc, uint8 o) override; // returns new opcode
	void  triggerNmi() override;
};
