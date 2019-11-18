/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#ifndef CURSORJOY_H
#define CURSORJOY_H

#include "Joy.h"


// WoS:
// A cursor joystick interfaces maps to keys 5 (left), 6 (down), 7 (up), 8 (right) and 0 (fire).
// Reading a cursor joystick thus requires a combination of bit 4 of port 0xf7fe and bits 0, 2, 3 and 4 of port 0xeffe.
// Common interfaces offering a cursor joystick option included those produced by Protek and AGF.

class CursorJoy : public Joy
{
public:
	explicit CursorJoy(Machine*);

protected:
	CursorJoy(Machine*, isa_id);

	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
};


class ProtekJoy : public CursorJoy
{
	// Kio: The PCB contains two 74LS32 quad OR and one 74LS09 quad AND with oK.

public:
	explicit ProtekJoy(Machine* m)		: CursorJoy(m,isa_ProtekJoy) {}
};


#endif
















