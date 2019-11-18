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


#ifndef MACHINEZX80_H
#define MACHINEZX80_H

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

#endif
