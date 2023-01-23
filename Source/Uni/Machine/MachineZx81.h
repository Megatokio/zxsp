#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Machine.h"
#include "ZxInfo/info.h"
#include "Ula/UlaZx81.h"
#include "Ula/MmuZx81.h"


class MachineZx81 : public Machine
{
protected:
	MachineZx81(MachineController* parent, isa_id id, Model);

	bool handleSaveTapePatch() override;
	bool handleLoadTapePatch() override;

public:
	explicit MachineZx81(MachineController*);

	int32	nmiAtCycle(int32 cc_nmi) override		{ return UlaZx81Ptr(ula)->nmiAtCycle(cc_nmi); }

	void	loadP81(FD&, bool p81) throws override;
	void	saveP81(FD&, bool p81) throws override;
};


