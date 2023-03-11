#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Machine.h"


class MachineZx80 : public Machine
{
protected:
	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;

public:
	MachineZx80(IMachineController*, bool is60hz = false);

	void loadO80(FD&) override;
	void saveO80(FD&) override;
};
