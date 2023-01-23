#pragma once
// Copyright (c) 1996 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "Z80/goodies/z80_goodies.h"


//enum { LegalOpcode, IllegalOpcode, WeirdOpcode };


class Z80_Disassembler
{
public:
	uint8*	core;

private:
	cstr expand_word(uint8 n, uint &ip) const;

public:
	Z80_Disassembler()                      :core(NULL){}
	explicit Z80_Disassembler(uint8* core)	:core(core){}
	virtual ~Z80_Disassembler()             {}

	virtual uint8 peek(uint ip) const		{ return core[uint16(ip)]; }

	cstr	disassemble     (uint ip) const;
	int		opcodeLength    (uint ip) const;
	int		opcodeLegalState(uint ip) const;
	cstr    opcodeMnemo     (uint ip) const;
};





















