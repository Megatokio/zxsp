#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
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


/*	Keymap as used by Keyboard:
*/

union Keymap
{
	uint8		row[8];				// keyboard matrix as seen by ula
	int64		allrows;

	Keymap()	:allrows(-1LL){}

	uint8&	operator[](int i)	{ return row[i]; }
	void	clear(uint8 n=0xFF)	{ allrows |= n * 0x0101010101010101ull; }
	bool	keyPressed()		{ return allrows != -1ll; }
	bool	noKeyPressed()		{ return allrows == -1ll; }

	void	res_key(uint8 spec) { row[(spec>>4)&7] |= 1<<(spec&7); }
	void	set_key(uint8 spec) { row[(spec>>4)&7] &= ~(1<<(spec&7)); }
	bool	get_key(uint8 spec) { return !(row[(spec>>4)&7] & (1<<(spec&7))); }
};










