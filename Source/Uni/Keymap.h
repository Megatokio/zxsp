/*	Copyright  (c)	Günter Woigk 2015 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef KEYMAP_H
#define KEYMAP_H
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


#endif







