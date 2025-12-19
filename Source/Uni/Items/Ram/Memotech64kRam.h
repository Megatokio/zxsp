// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "ExternalRam.h"


namespace zxsp
{

class Memotech64kRam : public ExternalRam
{
	uint dip_switches; // bits[3…0] == dip switch[1…4]

public:
	enum DipSwitches { A = 0x8, B = 0x4, C = 0x2, D = 0x1, E = 0x6 }; // lt. Memopak manual

	Memotech64kRam(Machine*, uint dip_switches = E);

	void setDipSwitches(uint);
	uint getDipSwitches() const volatile { return dip_switches; }

	static bool isValidDipSwitches(uint sw) { return sw == A || sw == B || sw == C || sw == D || sw == E; }

protected:
	~Memotech64kRam() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;

private:
	void map_dip_switched_ram();
};

} // namespace zxsp
