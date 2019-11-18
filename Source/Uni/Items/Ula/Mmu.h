#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "Memory.h"
#include "Item.h"
#include "zxsp_types.h"


class Mmu : public Item
{
protected:
	Z80*		cpu;
	union
	{
	Ula*		ula;
	UlaZxsp*	ula_zxsp;
	};
	MemoryPtr	ram;
	MemoryPtr	rom;

public:
VIR void	mapMem() {}		// map memory in models which have expandable memory

// RAMDIS und ROMDIS behandeln, falls vorhanden:
// Hier sind die Dummy-Funktionen, falls ein Gerät das jeweilige Signal nicht hat.
// z.B.: ZX80 hat kein ROMDIS, Speccies haben kein RAMDIS
// Sonst muss es in der jeweiligen MMU reimplementiert sein.
// ramCS() und romCS() der MMU dürfen prev()->ramCS()/prev()->romCS() nicht mehr aufrufen!
// The calling chains for ramCS and romCS end here (Mmu).
// Note: dass wir hier ramcs/romcs aktualisieren, ist überflüssig.
// Note: ramCS() wird z.Zt. gar nicht benutzt, da Ram-Erweiterungen z.Zt. das interne Ram des Computers erweitern.
// Note: +3/+2A: statt romCS1 und romCS2 wird romCS benutzt
//
VIR void    ramCS(bool f) override { ramdis_in=f; logline("Mmu.ramCS(%i)",f); }  // ZX80, ZX81
VIR void    romCS(bool f) override { romdis_in=f; logline("Mmu.romCS(%i)",f); }  // ZX81, ZXSP, ZX128, +2

/*  note:   RAMCS:  input '1':    activated: disable internal Ram (ZXSP)
					input high Z: not activated
			ROMCS:  input '1':    activated: disable internal Rom (ZXSP,128,+2)
					input high Z: not activated
			ROMCS1: input '1':    activated: disable presumably lower internal Rom (ZX+3/+2A)
					input high Z: not activated
			ROMCS2: input '1':    activated: disable presumably upper internal Rom (ZX+3/+2A)
					input high Z: not activated

	i can't imagine a scenario where setting CS1 and CS2 differently makes sense.
	ROMCS is used for +3/+2A for simplicity as well.

	Pageing of the internal ram and rom (if applicable) is handled by Item::Mmu.
	In general items should not actively unmap their memory as result of incoming ROMCS.
	Instead they should trust that the sender will map it's rom into the Z80's memory space.
	While their backside ROMCS is active, items must not page in their memory, just store values written to their registers only.
*/

VIR bool	hasPort7ffd() volatile const noexcept	{ return no; }
VIR void	setPort7ffd(uint8)						{ /*nop*/ }
VIR uint8	getPort7ffd() volatile const			{ return 0xFF; }

VIR bool	hasPort1ffd() volatile const noexcept	{ return no; }
VIR void	setPort1ffd(uint8)						{ /*nop*/ }
VIR uint8   getPort1ffd() volatile const			{ return 0xFF; }

VIR bool	hasPortF4() volatile const noexcept		{ return no; }
VIR void	setPortF4(uint8)						{ /*nop*/ }
VIR uint8	getPortF4() volatile const				{ return 0xFF; }

protected:
	Mmu(Machine*, isa_id, cstr o_addr, cstr i_addr);

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	//void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	//void	saveToFile		(FD&) const	throws override;
	//void	loadFromFile	(FD&) throws override;
	uint8	handleRomPatch	(uint16, uint8 o) override				{ return o; }	// returns opcode read
	uint8	readMemory		(Time, int32, uint16, uint8 n)override	{ return n; }	// returns byte read
	void	writeMemory		(Time, int32, uint16, uint8) override	{}
};












