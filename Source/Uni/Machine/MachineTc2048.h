#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineZxsp.h"


class MachineTc2048 : public MachineZxsp
{
protected:
	MachineTc2048(IMachineController*, Model, isa_id);

public:
	explicit MachineTc2048(IMachineController*);

	void loadScr(FD&) override;
};
