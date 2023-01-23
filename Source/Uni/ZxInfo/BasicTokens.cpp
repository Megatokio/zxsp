// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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


























