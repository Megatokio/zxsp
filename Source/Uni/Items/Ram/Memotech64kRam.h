#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"


class Memotech64kRam : public ExternalRam
{
	uint    dip_switches;           // bits[3…0] == dip switch[1…4]

public:
	explicit Memotech64kRam(Machine*);
	virtual ~Memotech64kRam();

	void    setDipSwitches(uint);
	uint	getDipSwitches() volatile const 	{ return dip_switches; }

protected:
	// Item interface:
	void	powerOn(/*t=0*/ int32 cc) override;

private:
	void	map_dip_switched_ram();
};





