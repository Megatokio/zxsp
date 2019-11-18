#pragma once
/*	Copyright  (c)	Günter Woigk 1996 - 2019
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

#include "kio/kio.h"


enum { LegalOpcode, IllegalOpcode, WeirdOpcode };


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





















