#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Machine.h"


class MachineJupiter : public Machine
{
protected:
	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;
	void loadAce(FD&) override;
	void saveAce(FD&) override;

public:
	explicit MachineJupiter(gui::MachineController*);
};
