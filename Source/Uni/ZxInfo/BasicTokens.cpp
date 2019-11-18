/*	Copyright  (c)	Günter Woigk 1994 - 2019
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
#include "ZxInfo.h"



/*	0x90 "A" - 0xa4 "U" are UDGs.
	"SPECTRUM" and "PLAY" are new tokens on 128++ replacing UDG "T" and "U"
*/
cstr basic_token[] =
{
/* 0x80 */	" ", "▝", "▘", "▀", "▗", "▐", "▚", "▜", "▖", "▞", "▌", "▛", "▄", "▟", "▙", "█",
/* 0x90 */	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
/* 0xa0 */	"Q", "R", "S", "SPECTRUM", "PLAY", "RND", "INKEY$", "PI", "FN", "POINT", "SCREEN$", "ATTR", "AT", "TAB", "VAL$", "CODE",
/* 0xb0 */	"VAL", "LEN", "SIN", "COS", "TAN", "ASN", "ACS", "ATN", "LN", "EXP", "INT", "SQR", "SGN", "ABS", "PEEK", "IN",
/* 0xc0 */	"USR", "STR$", "CHR$", "NOT", "BIN", "OR", "AND", "<=", ">=", "<>", "LINE", "THEN", "TO", "STEP", "DEF FN", "CAT",
/* 0xd0 */	"FORMAT", "MOVE", "ERASE", "OPEN #", "CLOSE #", "MERGE", "VERIFY", "BEEP",
			"CIRCLE", "INK", "PAPER", "FLASH", "BRIGHT", "INVERSE", "OVER", "OUT",
/* 0xe0 */	"LPRINT", "LLIST", "STOP", "READ", "DATA", "RESTORE", "NEW", "BORDER",
			"CONTINUE", "DIM", "REM", "FOR", "GO TO", "GO SUB", "INPUT", "LOAD",
/* 0xf0 */	"LIST", "LET", "PAUSE", "NEXT", "POKE", "PRINT", "PLOT", "RUN",
			"SAVE", "RANDOMIZE", "IF", "CLS", "DRAW", "CLEAR", "RETURN", "COPY"
};


























