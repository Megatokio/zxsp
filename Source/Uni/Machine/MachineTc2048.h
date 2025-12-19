// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "MachineZxsp.h"


namespace zxsp
{

class MachineTc2048 : public MachineZxsp
{
protected:
	MachineTc2048(IMachineController*, Model, isa_id);

public:
	explicit MachineTc2048(IMachineController*);

	void loadScr(FD&) override;
};

} // namespace zxsp
