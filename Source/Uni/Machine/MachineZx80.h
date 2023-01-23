#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Machine.h"
#include "ZxInfo/info.h"
#include "Ula/UlaZx80.h"
#include "Ula/MmuZx80.h"
#include "Keyboard.h"


class MachineZx80 : public Machine
{
protected:
	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;

public:
	explicit MachineZx80( MachineController*, Model=zx80, isa_id id=isa_MachineZx80 );

	void loadO80(FD&) throws override;
	void saveO80(FD&) throws override;

};


