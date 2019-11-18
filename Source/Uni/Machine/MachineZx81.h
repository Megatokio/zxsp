#pragma once
/*	Copyright  (c)	Günter Woigk 2008 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

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


