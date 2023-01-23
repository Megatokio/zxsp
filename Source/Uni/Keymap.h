#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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










