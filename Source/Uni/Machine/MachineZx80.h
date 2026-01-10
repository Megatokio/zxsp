// Copyright (c) 2008 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Machine.h"


namespace zxsp
{

class MachineZx80 : public Machine
{
protected:
	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;

public:
	MachineZx80(IMachineController*, IScreen*, bool is60hz = false);

	void loadO80(FD&) override;
	void saveO80(FD&) override;
};

} // namespace zxsp
