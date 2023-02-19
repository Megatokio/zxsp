// Copyright (c) 1994 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include "Keyboard.h"
#include "Machine.h"
#include "MachineController.h"
#include "Ula/Ula.h"
#include "ZxInfo.h"


// Modifier key masks for sticky keys on visual keyboard:
#define capsshiftsticky	   0x0001 // caps shift key no longer pressed but still considered down
#define capsshift2sticky   0x0002 // for models with 2 caps shift keys
#define symbolshiftsticky  0x0004 // symbol shift key no longer pressed but still considered down
#define symbolshift2sticky 0x0008 // for models with 2 caps shift keys

enum ZxspKey : uint8 {
	//	Names for Keys on the ZXSP and ZXSP+ keyboard.
	//	Compound keys on ZXSP+ which connect two matrix points
	//	are assigned virtual positions with CCC = 5…7
	//
	//	zxkeycode = %cRRRsCCC
	//		c = add caps shift key
	//		s = add symbol shift key
	//		RRR = row: 0..7
	//		CCC = col: 0..4 = normal keys, 5..7 = compound keys

	x  = 0x77, // no mapping
	c  = 0x80, // add caps shift key
	s  = 0x08, // add symbol shift key
	up = 0x80, // key-up marker for BtZxKbd

	// clang-format off
	
	CSH = 0x00,	Z,	X,	C,	V,	/*	left bottom row	*/	cZ1, cZ2, cZ3,	/*	EDIT, CAPS LOCK, TRUE VIDEO		*/
	A = 0x10,	S,	D,	F,	G,	/* 	...				*/ 	cZ4, cZ5, cZ6,	/*	INV.VIDEO, CRSR LEFT, CRSR DOWN	*/
	Q = 0x20,	W,	E,	R,	T,	/*	...				*/ 	cZ7, cZ8, cZ9,	/*	CRSR UP, CRSR RIGHT, GRAPHICS	*/
	Z1 = 0x30,	Z2,	Z3,	Z4,	Z5, /* left top row		*/ 	cZ0, cSPC,		/*	DELETE, BREAK,					*/
	Z0 = 0x40,	Z9,	Z8,	Z7,	Z6,	/* right top row	*/ 	sO, sP,			/*	; "								*/
	P = 0x50,	O,	I,	U,	Y,	/* 	...				*/ 	sN,	sM,			/*	, .								*/
	ENT = 0x60,	L,	K,	J,	H,	/*	...				*/	CSH2,SSH2, EXT,	/*	2nd CSH, 2nd SSH, EXTENDED MODE	*/
	SPC = 0x70,	SSH, M,	N,	B,	/* right bottom row	*/	NOKEY = 0x77

	// clang-format on
};

static const uint8 decomposed_zxplus[64] = {
	//	ZxspKey decomposition for ZX Spectrum+ keys
	//
	//	decompose composed key:
	//	composed keys are virtual ZxspKeys with column = 5…7 from the above table
	//
	//	This table decomposes the ZxspKey codes from the above table
	//	into the major key scancode and for compound keys the modifier key scancode.
	//	Most simple keys map 1:1.  note: index = ((c&0x70)>>1) + (c&0x07)
	//
	//	zxkeycode = %cRRRsCCC
	//		c = add caps shift key
	//		s = add symbol shift key
	//		RRR = row: 0..7
	//		CCC = col: 0..4 = normal keys, 5..7 = special codes for compound keys

	c + x, Z,	  X,  C,  V,  c + Z1, c + Z2,  c + Z3,	  /*	EDIT, CAPS LOCK, TRUE VIDEO		*/
	A,	   S,	  D,  F,  G,  c + Z4, c + Z5,  c + Z6,	  /*	INV.VIDEO, CRSR LEFT, CRSR DOWN	*/
	Q,	   W,	  E,  R,  T,  c + Z7, c + Z8,  c + Z9,	  /*	CRSR UP, CRSR RIGHT, GRAPHICS	*/
	Z1,	   Z2,	  Z3, Z4, Z5, c + Z0, c + SPC, x,		  /*	DELETE, BREAK,					*/
	Z0,	   Z9,	  Z8, Z7, Z6, s + O,  s + P,   x,		  /*	; "								*/
	P,	   O,	  I,  U,  Y,  s + N,  s + M,   x,		  /*	, .								*/
	ENT,   L,	  K,  J,  H,  c + x,  s + x,   c + s + x, /*	2nd CSH, 2nd SSH, EXTENDED MODE	*/
	SPC,   s + x, M,  N,  B,  x,	  x,	   x		  /*    -x- -x- NOKEY					*/
};

static const AsciiToZxkeyMap ascii2zxkey_zxsp = {
	//	Ascii char code -> ZxspKey key code
	//  40-key (rubber) model with no composed keys
	//
	//	logical translation for text input, e.g. Basic
	//	all characters should be translated, if possible
	//	control codes are handled by fallback to physical mapping
	//
	//	[]|{}~ are mapped to Symbol Shift + Key
	//	use CSH+SSH before these keys to enter EXTENDED keyboard MODE (doesn't work on German kbd wg. ALT)
	//	"£" is not part of this map, because it is non-ascii. (unicode≥128)

	x,		x,		x,		x,		 x,		 x,		 x,		 x,		 // 0x00-0x07: • • •  •  •  •  • •
	x,		x,		x,		x,		 x,		 ENT,	 x,		 x,		 // 0x08-0x0F: • • •  •  • RET • •
	x,		x,		Z5 + c, Z7 + c,	 Z8 + c, Z6 + c, x,		 x,		 // 0x10-0x17: • • ←  ↑  ➞  ↓  • •
	x,		x,		x,		SPC + c, x,		 x,		 x,		 x,		 // 0x18-0x1F: • • • ESC •  •  • •
	SPC,	Z1 + s, P + s,	Z3 + s,	 Z4 + s, Z5 + s, Z6 + s, Z7 + s, // 0x20-0x27: SPC ! " # $ % & '
	Z8 + s, Z9 + s, B + s,	K + s,	 N + s,	 J + s,	 M + s,	 V + s,	 // 0x28-0x2F: ()*+,-./
	Z0,		Z1,		Z2,		Z3,		 Z4,	 Z5,	 Z6,	 Z7,	 // 0x30-0x37: 01234567
	Z8,		Z9,		Z + s,	O + s,	 R + s,	 L + s,	 T + s,	 C + s,	 // 0x38-0x3F: 89:;<=>?
	Z2 + s, A + c,	B + c,	C + c,	 D + c,	 E + c,	 F + c,	 G + c,	 // 0x40-0x47: @ABCDEFG
	H + c,	I + c,	J + c,	K + c,	 L + c,	 M + c,	 N + c,	 O + c,	 // 0x48-0x4F: HIJKLMNO
	P + c,	Q + c,	R + c,	S + c,	 T + c,	 U + c,	 V + c,	 W + c,	 // 0x50-0x57: PQRSTUVW
	X + c,	Y + c,	Z + c,	Y + s,	 D + s,	 U + s,	 H + s,	 Z0 + s, // 0x58-0x5F: XYZ[\]^_
	x,		A,		B,		C,		 D,		 E,		 F,		 G,		 // 0x60-0x67: `abcdefg
	H,		I,		J,		K,		 L,		 M,		 N,		 O,		 // 0x68-0x6F: hijklmno
	P,		Q,		R,		S,		 T,		 U,		 V,		 W,		 // 0x70-0x77: pqrstuvw
	X,		Y,		Z,		F + s,	 S + s,	 G + s,	 A + s,	 Z0 + c	 // 0x78-0x7F: x y z { | } ~ DEL
};

static const OskeyToZxkeyMap oskey2zxkey_zxsp = {
	//	Mac OSX keycode -> ZxspKey key code
	//  40-key (rubber) model with no composed keys
	//
	//	physical translation for games
	//	all 40 keys are mapped acc. to their position
	//	other keys are handled by fallback to logical translation

	A,	 S,	  D,   F,	H,	 G,	  Z,   X,	// 0x00-0x07:  a   s   d   f   h   g   z   x
	C,	 V,	  x,   B,	Q,	 W,	  E,   R,	// 0x08-0x0F:  c   v   ^   b   q   w   e   r
	Y,	 T,	  Z1,  Z2,	Z3,	 Z4,  Z6,  Z5,	// 0x10-0x17:  y   t   1   2   3   4   6   5
	x,	 Z9,  Z7,  x,	Z8,	 Z0,  x,   O,	// 0x18-0x1F:  •   9   7   ß   8   0   +   O
	U,	 x,	  I,   P,	ENT, L,	  J,   x,	// 0x20-0x27:  u   ü   i   p  RET  l   j   ä
	K,	 ENT, x,   SSH, x,	 N,	  M,   SPC, // 0x28-0x2F:  k   ö   #   ,   -   n   m   .
	x,	 SPC, CSH, x,	x,	 x,	  x,   x,	// 0x30-0x37: TAB SPC  <  DEL  •  ESC  •   •
	CSH, x,	  CSH, SSH, CSH, CSH, SSH, x,	// 0x38-0x3F: CSH  •  ALT CTL CSH2ALT2CTL2 •

	// x,x,x,x,x,x,x,x,						// 0x40-0x47:  •  [.]  •  [*]  •  [+] •   •
	// x,x,x,x,x,x,x,x,						// 0x48-0x4F:  •   •   •  [/][RET] • [-]  •
	// x,x,x,x,x,x,x,x,						// 0x50-0x57:  •  [=] [0] [1] [2] [3] [4] [5]
	// x,x,x,x,x,x,x,x,						// 0x58-0x5F: [6] [7]  •  [8] [9]  •
	// •   • x,x,x,x,x,x,x,x,				// 0x60-0x67:
	// x,x,x,x,x,x,x,x, 					// 0x68-0x6F:
	// x,x,x,x,x,x,x,x, 					// 0x70-0x77:
	// x,x,x,x,x,x,x,x,						// 0x78-0x7F:  •   •   •   ←   ➞   ↓  ↑   •
};

static const AsciiToZxkeyMap ascii2zxkey_zxplus = {
	//	Ascii char code -> ZxspKey key code
	//  "plus" models with composed keys
	//
	//	logical translation for text input, e.g. Basic
	//	all characters should be translated, if possible
	//	control codes are handled by fallback to physical mapping
	//	composed keycodes where possible.
	//
	//	[]|{}~ are mapped to Symbol Shift + Key
	//	use CSH+SSH before these keys to enter EXTENDED keyboard MODE (doesn't work on German kbd wg. ALT)
	//	"£" is not part of this map, because it is non-ascii. (unicode≥128)

	x,		x,		x,	   x,	   x,	   x,	   x,	   x,	   // 0x00-0x07: • • •  •  •  •  • •
	x,		x,		x,	   x,	   x,	   ENT,	   x,	   x,	   // 0x08-0x0F: • • •  •  • RET • •
	x,		x,		cZ5,   cZ7,	   cZ8,	   cZ6,	   x,	   x,	   // 0x10-0x17: • • ←  ↑  ➞  ↓  • •
	x,		x,		x,	   cSPC,   x,	   x,	   x,	   x,	   // 0x18-0x1F: • • • ESC •  •  • •
	SPC,	Z1 + s, sP,	   Z3 + s, Z4 + s, Z5 + s, Z6 + s, Z7 + s, // 0x20-0x27: space ! " # $ % & '
	Z8 + s, Z9 + s, B + s, K + s,  sN,	   J + s,  sM,	   V + s,  // 0x28-0x2F: ()*+,-./
	Z0,		Z1,		Z2,	   Z3,	   Z4,	   Z5,	   Z6,	   Z7,	   // 0x30-0x37: 01234567
	Z8,		Z9,		Z + s, sO,	   R + s,  L + s,  T + s,  C + s,  // 0x38-0x3F: 89:;<=>?
	Z2 + s, A + c,	B + c, C + c,  D + c,  E + c,  F + c,  G + c,  // 0x40-0x47: @ABCDEFG
	H + c,	I + c,	J + c, K + c,  L + c,  M + c,  N + c,  O + c,  // 0x48-0x4F: HIJKLMNO
	P + c,	Q + c,	R + c, S + c,  T + c,  U + c,  V + c,  W + c,  // 0x50-0x57: PQRSTUVW
	X + c,	Y + c,	Z + c, Y + s,  D + s,  U + s,  H + s,  Z0 + s, // 0x58-0x5F: XYZ[\]^_
	x,		A,		B,	   C,	   D,	   E,	   F,	   G,	   // 0x60-0x67: `abcdefg
	H,		I,		J,	   K,	   L,	   M,	   N,	   O,	   // 0x68-0x6F: hijklmno
	P,		Q,		R,	   S,	   T,	   U,	   V,	   W,	   // 0x70-0x77: pqrstuvw
	X,		Y,		Z,	   F + s,  S + s,  G + s,  A + s,  cZ0	   // 0x78-0x7F: xyz{|}~<del>
};

static const OskeyToZxkeyMap oskey2zxkey_zxplus = {
	// Mac OSX keycode -> ZxspKey key code
	// "plus" models with composed keys
	//
	// physical translation for games
	// only the main 36 keys (A-Z, 0-9) and ENTER are mapped acc. to their position
	// other keys are handled by fallback to logical translation
	// composed keys are never mapped.

	A,	 S,	  D,   F,	H,	  G,	Z,	  X,   // 0x00-0x07:  a   s   d   f   h   g   z   x
	C,	 V,	  x,   B,	Q,	  W,	E,	  R,   // 0x08-0x0F:  c   v   ^   b   q   w   e   r
	Y,	 T,	  Z1,  Z2,	Z3,	  Z4,	Z6,	  Z5,  // 0x10-0x17:  y   t   1   2   3   4   6   5
	x,	 Z9,  Z7,  x,	Z8,	  Z0,	x,	  O,   // 0x18-0x1F:  •   9   7   ß   8   0   •   O
	U,	 ENT, I,   P,	ENT,  L,	J,	  ENT, // 0x20-0x27:  u   ü   i   p  RET  l   j   ä
	K,	 ENT, x,   x,	x,	  N,	M,	  x,   // 0x28-0x2F:  k   ö   #   ,   -   n   m   .
	x,	 SPC, x,   cZ0, x,	  cSPC, x,	  x,   // 0x30-0x37:  •  SPC  <  DEL  •  ESC  •   •
	CSH, x,	  CSH, SSH, CSH2, CSH2, SSH2, x,   // 0x38-0x3F: CSH  •  ALT CTL CSH2ALT2CTL2 •

	// x,x,x,x,x,x,x,x,							// 0x40-0x47:  •  [.]  • [*] •  [+]  •    •
	// x,x,x,x,x,x,x,x,							// 0x48-0x4F:  •   •   • [/][RET] •  [-]  •
	// x,x,x,x,x,x,x,x,							// 0x50-0x57:  •  [=] [0] [1] [2] [3] [4] [5]
	// x,x,x,x,x,x,x,x,							// 0x58-0x5F: [6] [7]  •  [8] [9]  •   •   •
	// x,x,x,x,x,x,x,x,							// 0x60-0x67:
	// x,x,x,x,x,x,x,x,							// 0x68-0x6F:
	// x,x,x,x,x,x,x,x, 						// 0x70-0x77:
	// x,x,x,x,x,x,x,x,							// 0x78-0x7F:  •   •   •   ← ➞   ↓   ↑   •
};

static const AsciiToZxkeyMap ascii2zxkey_timex = {
	//	Ascii char code -> ZxspKey key code
	//  Timex models with 2nd CAPS SHIFT and one composed key: BREAK
	//	logical translation for text input, e.g. Basic

	x,		x,		x,		x,		x,		x,		x,		x,		// 0x00-0x07: • • •  •  •  •  • •
	x,		x,		x,		x,		x,		ENT,	x,		x,		// 0x08-0x0F: • • •  •  • RET • •
	x,		x,		Z5 + c, Z7 + c, Z8 + c, Z6 + c, x,		x,		// 0x10-0x17: • • ←  ↑  ➞  ↓  • •
	x,		x,		x,		cSPC,	x,		x,		x,		x,		// 0x18-0x1F: • • • ESC •  •  • •
	SPC,	Z1 + s, P + s,	Z3 + s, Z4 + s, Z5 + s, Z6 + s, Z7 + s, // 0x20-0x27: SPC ! " # $ % & '
	Z8 + s, Z9 + s, B + s,	K + s,	N + s,	J + s,	M + s,	V + s,	// 0x28-0x2F: ()*+,-./
	Z0,		Z1,		Z2,		Z3,		Z4,		Z5,		Z6,		Z7,		// 0x30-0x37: 01234567
	Z8,		Z9,		Z + s,	O + s,	R + s,	L + s,	T + s,	C + s,	// 0x38-0x3F: 89:;<=>?
	Z2 + s, A + c,	B + c,	C + c,	D + c,	E + c,	F + c,	G + c,	// 0x40-0x47: @ABCDEFG
	H + c,	I + c,	J + c,	K + c,	L + c,	M + c,	N + c,	O + c,	// 0x48-0x4F: HIJKLMNO
	P + c,	Q + c,	R + c,	S + c,	T + c,	U + c,	V + c,	W + c,	// 0x50-0x57: PQRSTUVW
	X + c,	Y + c,	Z + c,	Y + s,	D + s,	U + s,	H + s,	Z0 + s, // 0x58-0x5F: XYZ[\]^_
	x,		A,		B,		C,		D,		E,		F,		G,		// 0x60-0x67: `abcdefg
	H,		I,		J,		K,		L,		M,		N,		O,		// 0x68-0x6F: hijklmno
	P,		Q,		R,		S,		T,		U,		V,		W,		// 0x70-0x77: pqrstuvw
	X,		Y,		Z,		F + s,	S + s,	G + s,	A + s,	Z0 + c	// 0x78-0x7F: x y z { | } ~ DEL
};

static const OskeyToZxkeyMap oskey2zxkey_timex = {
	// Timex models with 2nd CAPS SHIFT and one composed key: BREAK
	//
	// Mac OSX keycode -> ZxspKey key code
	// physical translation for games
	// all 41 keys (incl. BREAK) are mapped
	// other keys are handled by fallback to logical translation

	A,	 S,	  D,   F,	H,	  G,	Z,	 X,	   // 0x00-0x07:  a   s   d   f   h   g   z   x
	C,	 V,	  x,   B,	Q,	  W,	E,	 R,	   // 0x08-0x0F:  c   v   ^   b   q   w   e   r
	Y,	 T,	  Z1,  Z2,	Z3,	  Z4,	Z6,	 Z5,   // 0x10-0x17:  y   t   1   2   3   4   6   5
	x,	 Z9,  Z7,  x,	Z8,	  Z0,	x,	 O,	   // 0x18-0x1F:  •   9   7   ß   8   0   •   O
	U,	 x,	  I,   P,	ENT,  L,	J,	 ENT,  // 0x20-0x27:  u   ü   i   p  RET  l   j   ä
	K,	 ENT, x,   SSH, CSH2, N,	M,	 cSPC, // 0x28-0x2F:  k   ö   #   ,   -   n   m   .
	x,	 SPC, CSH, x,	x,	  x,	x,	 x,	   // 0x30-0x37:  •  SPC  <  DEL  •  ESC  •   •
	CSH, x,	  CSH, SSH, CSH2, CSH2, SSH, x,	   // 0x38-0x3F: CSH  •  ALT CTL CSH2ALT2CTL2 •

	// x,x,x,x,x,x,x,x,							// 0x40-0x47:  •  [.]  •  [*]  •  [+]  •  •
	// x,x,x,x,x,x,x,x,							// 0x48-0x4F:  •   •   •  [/][RET] •  [-] •
	// x,x,x,x,x,x,x,x,							// 0x50-0x57:  •  [=] [0] [1] [2] [3] [4] [5]
	// x,x,x,x,x,x,x,x,							// 0x58-0x5F: [6] [7]  •  [8] [9]  •   •  •
	// x,x,x,x,x,x,x,x,							// 0x60-0x67:
	// x,x,x,x,x,x,x,x, 						// 0x68-0x6F:
	// x,x,x,x,x,x,x,x, 						// 0x70-0x77:
	// x,x,x,x,x,x,x,x,							// 0x78-0x7F:  •   •   •   ←   ➞   ↓  ↑  •
};

static const AsciiToZxkeyMap ascii2zxkey_zx81 = {
	// ZX80, ZX81 and clones.
	// ZX80: mappings for '*' and '"' are different and are fixed in c'tor
	// no composed keys.

	x,	   x,	  x,	  x,	  x,		x,		x,	   x,	   // 0x00-0x07: • • •  •  •  •  • •
	x,	   x,	  x,	  x,	  x,		ENT,	x,	   x,	   // 0x08-0x0F: • • •  •  • RET • •
	x,	   x,	  Z5 + c, Z7 + c, Z8 + c,	Z6 + c, x,	   x,	   // 0x10-0x17: • • ←  ↑  ➞  ↓  • •
	x,	   x,	  x,	  SPC,	  x,		x,		x,	   x,	   // 0x18-0x1F: • • • ESC •  •  • •
	SPC,   x,	  P + c,  x,	  U + c,	x,		x,	   x,	   // 0x20-0x27: space ! " # $ % & '
	I + c, O + c, B + c,  K + c,  0x71 + c, J + c,	0x71,  V + c,  // 0x28-0x2F: ()*+,-./
	Z0,	   Z1,	  Z2,	  Z3,	  Z4,		Z5,		Z6,	   Z7,	   // 0x30-0x37: 01234567
	Z8,	   Z9,	  Z + c,  X + c,  N + c,	L + c,	M + c, C + c,  // 0x38-0x3F: 89:;<=>?
	x,	   A,	  B,	  C,	  D,		E,		F,	   G,	   // 0x40-0x47: @ABCDEFG
	H,	   I,	  J,	  K,	  L,		M,		N,	   O,	   // 0x48-0x4F: HIJKLMNO
	P,	   Q,	  R,	  S,	  T,		U,		V,	   W,	   // 0x50-0x57: PQRSTUVW
	X,	   Y,	  Z,	  x,	  x,		x,		H + c, x,	   // 0x58-0x5F: XYZ[\]^_
	x,	   A,	  B,	  C,	  D,		E,		F,	   G,	   // 0x60-0x67: `abcdefg
	H,	   I,	  J,	  K,	  L,		M,		N,	   O,	   // 0x68-0x6F: hijklmno
	P,	   Q,	  R,	  S,	  T,		U,		V,	   W,	   // 0x70-0x77: pqrstuvw
	X,	   Y,	  Z,	  x,	  x,		x,		x,	   Z0 + c, // 0x78-0x7F: xyz{|}~<del>
};

static const OskeyToZxkeyMap oskey2zxkey_zx81 = {
	// ZX80, ZX81 and clones.
	// ZX80: mappings for '*' and '"' are different and are fixed in c'tor
	// no composed keys.
	//
	// physical translation for games
	// all 40 keys are mapped acc. to their position
	// other keys are handled by fallback to logical translation

	A,	 S,	  D,   F,	H,	 G,	  Z,   X,	// 0x00-0x07:  a   s   d   f   h   g   z   x
	C,	 V,	  x,   B,	Q,	 W,	  E,   R,	// 0x08-0x0F:  c   v   ^   b   q   w   e   r
	Y,	 T,	  Z1,  Z2,	Z3,	 Z4,  Z6,  Z5,	// 0x10-0x17:  y   t   1   2   3   4   6   5
	x,	 Z9,  Z7,  x,	Z8,	 Z0,  x,   O,	// 0x18-0x1F:  •   9   7   ß   8   0   •   O
	U,	 x,	  I,   P,	ENT, L,	  J,   x,	// 0x20-0x27:  u   ü   i   p  RET  l   j   ä
	K,	 ENT, x,   SSH, x,	 N,	  M,   SPC, // 0x28-0x2F:  k   ö   #   ,   -   n   m   .
	x,	 SPC, CSH, x,	x,	 x,	  x,   x,	// 0x30-0x37:  •  SPC  <  DEL  •  ESC  •   •
	CSH, x,	  CSH, CSH, CSH, CSH, CSH, x,	// 0x38-0x3F: CSH  •  ALT CTL CSH2ALT2CTL2 •

	// x,x,x,x,x,x,x,x,						// 0x40-0x47:  •  [.]  •  [*]  •  [+]  •   •
	// x,x,x,x,x,x,x,x,						// 0x48-0x4F:  •   •   •  [/][RET] •  [-]  •
	// x,x,x,x,x,x,x,x,						// 0x50-0x57:  •  [=] [0] [1] [2] [3] [4] [5]
	// x,x,x,x,x,x,x,x,						// 0x58-0x5F: [6] [7]  •  [8] [9]  •   •   •
	// x,x,x,x,x,x,x,x,						// 0x60-0x67:
	// x,x,x,x,x,x,x,x, 					// 0x68-0x6F:
	// x,x,x,x,x,x,x,x, 					// 0x70-0x77:
	// x,x,x,x,x,x,x,x,						// 0x78-0x7F:  •   •   •   ←   ➞   ↓   ↑   •
};

static const AsciiToZxkeyMap ascii2zxkey_jupiter = {
	//	Ascii char code -> ZxspKey key code
	//  40-key Jupiter Ace with no composed keys
	//	logical translation for text input, e.g. Basic
	//
	//	like ZXSP but up and down keys are swapped
	//	and the matrix point of SSH is inserted left between CSH and Z
	//	and the matrix points for X to M are shifted right accordingly (--> fixed in update_keymap())

	x,		x,		x,		x,		 x,		 x,		 x,		 x,		 // 0x00-0x07: • • •  •  •  •  • •
	x,		x,		x,		x,		 x,		 ENT,	 x,		 x,		 // 0x08-0x0F: • • •  •  • RET • •
	x,		x,		Z5 + c, Z6 + c,	 Z8 + c, Z7 + c, x,		 x,		 // 0x10-0x17: • • ←  ↑  ➞  ↓  • •
	x,		x,		x,		SPC + c, x,		 x,		 x,		 x,		 // 0x18-0x1F: • • • ESC •  •  • •
	SPC,	Z1 + s, P + s,	Z3 + s,	 Z4 + s, Z5 + s, Z6 + s, Z7 + s, // 0x20-0x27: space ! " # $ % & '
	Z8 + s, Z9 + s, B + s,	K + s,	 N + s,	 J + s,	 M + s,	 V + s,	 // 0x28-0x2F: ()*+,-./
	Z0,		Z1,		Z2,		Z3,		 Z4,	 Z5,	 Z6,	 Z7,	 // 0x30-0x37: 01234567
	Z8,		Z9,		Z + s,	O + s,	 R + s,	 L + s,	 T + s,	 C + s,	 // 0x38-0x3F: 89:;<=>?
	Z2 + s, A + c,	B + c,	C + c,	 D + c,	 E + c,	 F + c,	 G + c,	 // 0x40-0x47: @ABCDEFG
	H + c,	I + c,	J + c,	K + c,	 L + c,	 M + c,	 N + c,	 O + c,	 // 0x48-0x4F: HIJKLMNO
	P + c,	Q + c,	R + c,	S + c,	 T + c,	 U + c,	 V + c,	 W + c,	 // 0x50-0x57: PQRSTUVW
	X + c,	Y + c,	Z + c,	Y + s,	 D + s,	 U + s,	 H + s,	 Z0 + s, // 0x58-0x5F: XYZ[\]^_
	x,		A,		B,		C,		 D,		 E,		 F,		 G,		 // 0x60-0x67: `abcdefg
	H,		I,		J,		K,		 L,		 M,		 N,		 O,		 // 0x68-0x6F: hijklmno
	P,		Q,		R,		S,		 T,		 U,		 V,		 W,		 // 0x70-0x77: pqrstuvw
	X,		Y,		Z,		F + s,	 S + s,	 G + s,	 A + s,	 Z0 + c	 // 0x78-0x7F: xyz{|}~<del>
};

static const OskeyToZxkeyMap oskey2zxkey_jupiter = {
	//	Mac OSX keycode -> ZxspKey key code
	//  40-key Jupiter Ace with no composed keys
	//	physical translation for games

	A,	 S,	  D,   F,	H,	 G,	  Z,   X,	// 0x00-0x07:  a   s   d   f   h   g   z   x
	C,	 V,	  x,   B,	Q,	 W,	  E,   R,	// 0x08-0x0F:  c   v   ^   b   q   w   e   r
	Y,	 T,	  Z1,  Z2,	Z3,	 Z4,  Z6,  Z5,	// 0x10-0x17:  y   t   1   2   3   4   6   5
	x,	 Z9,  Z7,  x,	Z8,	 Z0,  x,   O,	// 0x18-0x1F:  •   9   7   ß   8   0   •   O
	U,	 x,	  I,   P,	ENT, L,	  J,   x,	// 0x20-0x27:  u   ü   i   p  RET  l   j   ä
	K,	 ENT, x,   SSH, x,	 N,	  M,   SPC, // 0x28-0x2F:  k   ö   #   ,   -   n   m   .
	x,	 SPC, CSH, x,	x,	 x,	  x,   x,	// 0x30-0x37:  •  SPC  <  DEL  •  ESC  •   •
	CSH, x,	  CSH, SSH, CSH, CSH, SSH, x,	// 0x38-0x3F: CSH  •  ALT CTL CSH2ALT2CTL2 •

	// x,x,x,x,x,x,x,x,						// 0x40-0x47:  •  [.]  •  [*]  •  [+]  •   •
	// x,x,x,x,x,x,x,x,						// 0x48-0x4F:  •   •   •  [/][RET] •  [-]  •
	// x,x,x,x,x,x,x,x,						// 0x50-0x57:  •  [=] [0] [1] [2] [3] [4] [5]
	// x,x,x,x,x,x,x,x,						// 0x58-0x5F: [6] [7]  •  [8] [9]  •   •   •
	// x,x,x,x,x,x,x,x,						// 0x60-0x67:
	// x,x,x,x,x,x,x,x, 					// 0x68-0x6F:
	// x,x,x,x,x,x,x,x, 					// 0x70-0x77:
	// x,x,x,x,x,x,x,x,						// 0x78-0x7F:  •   •   •   ←   ➞   ↓   ↑   •
};

static const AsciiToZxkeyMap btzxkbd2zxkey_zxsp = {
	// ascii -> Zxsp key code
	// physical translation of ascii keycode for game mode on a "Recreated Bluetooth ZX Keyboard"

	x,		 x,		   x,		x,		x,		  x,	  x,		x,		  //
	x,		 x,		   x,		x,		x,		  x,	  x,		x,		  //
	x,		 x,		   x,		x,		x,		  x,	  x,		x,		  //
	x,		 x,		   x,		x,		x,		  x,	  x,		x,		  //
	x,		 SSH,	   x,		x,		CSH + up, SPC,	  x,		x,		  // 0x20-0x27: space ! " # $ % & '
	x,		 x,		   x,		x,		B,		  X,	  B + up,	N,		  // 0x28-0x2F: ()*+,-./
	J,		 J + up,   K,		K + up, L,		  L + up, ENT,		ENT + up, // 0x30-0x37: 01234567
	CSH,	 CSH + up, V + up,	V,		Z,		  X + up, Z + up,	N + up,	  // 0x38-0x3F: 89:;<=>?
	x,		 R,		   R + up,	T,		T + up,	  Y,	  Y + up,	U,		  // 0x40-0x47: @ABCDEFG
	U + up,	 I,		   I + up,	O,		O + up,	  P,	  P + up,	A,		  // 0x48-0x4F: HIJKLMNO
	A + up,	 S,		   S + up,	D,		D + up,	  F,	  F + up,	G,		  // 0x50-0x57: PQRSTUVW
	G + up,	 H,		   H + up,	C,		x,		  C + up, SPC + up, x,		  // 0x58-0x5F: XYZ[\]^_
	x,		 Z1,	   Z1 + up, Z2,		Z2 + up,  Z3,	  Z3 + up,	Z4,		  // 0x60-0x67: `abcdefg
	Z4 + up, Z5,	   Z5 + up, Z6,		Z6 + up,  Z7,	  Z7 + up,	Z8,		  // 0x68-0x6F: hijklmno
	Z8 + up, Z9,	   Z9 + up, Z0,		Z0 + up,  Q,	  Q + up,	W,		  // 0x70-0x77: pqrstuvw
	W + up,	 E,		   E + up,	M,		x,		  M + up, x,		x,		  // 0x78-0x7F: xyz{|}~<del>

}; // table must be filled to end with 'x'


// -----------------------------------------------------------
//				Constructor
// -----------------------------------------------------------

Keyboard::Keyboard(Machine* m, isa_id id, const AsciiToZxkeyMap cmap, const OskeyToZxkeyMap kmap) :
	Item(m, id, isa_Keyboard, internal, nullptr /*o_addr*/, nullptr /*i_addr*/),
	model(m->model),
	csh(CSH),
	ssh(SSH),
	csh2(NOKEY),
	ssh2(NOKEY),
	mode(kbdgame),
	modifiers(0),
	numkeys(0),
	stickykeys(0)
{
	memcpy(ascii2zxkey_map, cmap, sizeof(ascii2zxkey_map));
	memcpy(oskey2zxkey_map, kmap, sizeof(oskey2zxkey_map));
}

KeyboardZx81::KeyboardZx81(Machine* m, isa_id id) // ZX81 keyboard: ZX81, TS1000, TS1500, TK85
	:
	Keyboard(m, id, ascii2zxkey_zx81, oskey2zxkey_zx81)
{
	ssh = CSH;
}

KeyboardZx80::KeyboardZx80(Machine* m, isa_id id) // ZX80 keyboard
	:
	KeyboardZx81(m, id)
{
	// ZX80 keyboard:
	// same as ZX81 except '*' and '"'

	ascii2zxkey_map[0x22] = Y + c; // '"'
	ascii2zxkey_map[0x2a] = P + c; // '*'
}

KeyboardZxsp::KeyboardZxsp(Machine* m, isa_id id) // ZX Spectrum keyboard
	:
	Keyboard(m, id, ascii2zxkey_zxsp, oskey2zxkey_zxsp)
{}

KeyboardZxPlus::KeyboardZxPlus(Machine* m, isa_id id) // ZXplus keyboards: plus, Inves, 128, +2, +2A, +3
	:
	Keyboard(m, id, ascii2zxkey_zxplus, oskey2zxkey_zxplus)
{
	csh2 = CSH2;
	ssh2 = SSH2;
}

KeyboardTimex::KeyboardTimex(Machine* m, isa_id id) // Timex keyboards: TS2068 etc.
	:
	Keyboard(m, id, ascii2zxkey_timex, oskey2zxkey_timex)
{
	csh2 = CSH2;
}

KeyboardJupiter::KeyboardJupiter(Machine* m, isa_id id) // Jupiter Ace keyboard
	:
	Keyboard(m, id, ascii2zxkey_jupiter, oskey2zxkey_jupiter)
{
	// ssh = 0x01;
}


// -----------------------------------------------------------
//				Item Interface
// -----------------------------------------------------------

void Keyboard::powerOn(int32 cc)
{
	Item::powerOn(cc);
	assert(model == machine->model);
	allKeysUp();
}


// -----------------------------------------------------------
//				Keyboard Interface
// -----------------------------------------------------------

void Keyboard::allKeysUp()
{
	xlogIn("Keyboard::allKeysUp");

	modifiers  = 0;
	numkeys	   = 0;
	stickykeys = 0;

	keymap.clear();
	machine->ula->keymap.clear();
}

void Keyboard::setKbdMode(KbdMode newmode)
{
	xlogIn("Keyboard::setKbdMode");
	mode = newmode;
	allKeysUp();
}

void Keyboard::setKbdGame()
{
	xlogIn("Keyboard::setKbdGame");
	mode = kbdgame;
	allKeysUp();
}

void Keyboard::setKbdBtZxKbd()
{
	xlogIn("Keyboard::setKbdBtZxKbd");
	mode = kbdbtzxkbd;
	allKeysUp();
}

void Keyboard::setKbdBasic()
{
	xlogIn("Keyboard::setKbdBasic");
	mode = kbdbasic;
	allKeysUp();
}


/* --------------------------------------------------------------
	recalculate state of keyboard maps

	kbdType=kbdPhysical	=> physical mapping is preferred (for games)
	kbdType=kbdLogical	=> logical  mapping is preferred (for basic)

	the table of the preferred method is tried first. if it does not
	contain an entry for the pressed/released key, the other table is tried.

	"alt" works as auxiliary "caps shift" keys in both mapping modes.
	they are essential in logical mapping mode to get the "caps shift" + "number key",
	e.g. "caps shift" + "1" to edit a basic line. if you simply press "shift" plus "1"
	this is "!" and this is decoded as "symbol shift" + "1" which results in a "!"
	on the specci. on the other hand, "alt" + "1" works. useful for breaking into
	running basic programs too, since shift + space results in a simple space too.

	the state of control key is taken from ev->state in both modes, since it does
	not influence the character code returned by XLookupString().

	the state of shift and shift lock are taken from xev->state only in physical
	mapping mode. they are not decoded separately in logical mode, since the key
	translation table supplies the corresponding shift and control key state and an
	additional modifier detected will disturb. e.g. if you press "!" (that is: "shift" + "1")
	this results in "symbol shift" + "1" which is transposed to "!" by the specci
	keyboard routines just as expected. adding "caps shift" because "shift" is
	pressed on the real keyboard (just to get the "!") will result in "1" and both
	shift keys pressed to the specci, decoding to nothing (3 keys pressed!)

---------------------------------------------------------------*/

static void add_ghost_keys(Keymap& keymap)
{
	// add implicitely connected keys
	// if you press 3 keys where 2 are in the same row and 2 in the same column,
	// the resulting 4th key seems to be pressed too.

	uint8* e = &keymap[7];
	while (e > &keymap[0] && *e == 0xff) e--;
	if (e <= &keymap[0]) return; // only keys in 1 row or no keys pressed at all

p:
	for (uint8* a = &keymap[0]; a < e; a++) // loop over lines 1 to 7
	{
		if (*a == 0xff) continue; // no keys pressed in this row

		for (uint8* b = e; b > a; b--) // compare to other lines
		{
			uint8 xo = *a | *b;		  // xo = same bits 0 ((keys down)) in both rows
			if (xo == 0xff) continue; // no 2 keys in same column
			uint8 xa = *a & *b;		  // xa = all bits 0 ((keys down)) in any of both rows
			if (xa == xo) continue;	  // no 3rd key down

			// ghost key(s) detected:
			// rows a and b are connected via 2 keys on same column wire
			// => keys from a flow to b and vice versa

			*a = *b = xa;
			goto p; // ghost key may connect a or b to already tested wires
		}
	}
}

void KeyboardZxPlus::convert_to_matrix(Keymap& keymap)
{
	// decompose composed keys in keymap[]
	// only the ZXSP+ keyboards have composed keys
	//
	// sucht im Scancode-Buffer nach Komposittasten
	// und ersetzt diese durch die normale Tastenkombination,
	// wenn es diese Taste auf dem aktuellen Modell nicht gibt.
	// Aus BREAK wird so CAPS SHIFT + SPACE.

	for (int i = 0; i < 8; i++) // loop over all bytes in keymap[]
	{
		uint8 byte = keymap[i];			   // get byte from keymap[]
		if ((byte >> 5) == 0x07) continue; // no composed keys => next

		for (int j = 5; j < 8; j++) // loop over all composed keys in byte
		{
			if ((byte >> j) & 1) continue;				 // composed key not set => next
			uint8 decomp = decomposed_zxplus[i * 8 + j]; // decompose composed key
			if (decomp & s) keymap.set_key(ssh);		 // if s-flag is set then set SYMBOL SHIFT key
			if (decomp & c) keymap.set_key(csh);		 // if c-flag is set then set CAPS SHIFT key
			decomp &= 0x77;								 // remove c and s flags from key code
			if (decomp != NOKEY) keymap.set_key(decomp); // set base key
		}
	}
	keymap.clear(0xe0); // clear all composed keys in keymap[]
}

void KeyboardTimex::convert_to_matrix(Keymap& keymap)
{
	// Timex 2048/2068 keyboards have one composed key: BREAK = caps shift + space.

	if (keymap.get_key(cSPC))
	{
		keymap.set_key(CSH);
		keymap.set_key(SPC);
		keymap.res_key(cSPC);
	}
}

void KeyboardJupiter::convert_to_matrix(Keymap& keymap)
{
	// keys in the bottom row are reordered:
	// SSH was moved left next to CSH and all keys between shifted right one place

	uint row0 = keymap[0];
	uint row7 = keymap[7];
	keymap[0] = (row0 | ~0x01u /*CSH*/) & (row7 | ~0x02u /*SSH*/) & ((row0 << 1) | ~0x1Cu /*Z,X,C*/);
	keymap[7] = (row7 | ~0x01u /*SPC*/) & (row0 | ~0x10u /* V */) & ((row7 >> 1) | ~0x0Eu /*B,N,M*/);
}

void Keyboard::update_keymap()
{
	// update keymap[] (visual state of the virtual keyboard)
	// and ula.keymap[] (matrix as seen by Ula)

	Keymap newkeys;
	newkeys.clear();
	bool ownmeta = false; // set if keys supplied own caps/symbol flag

	for (uint k = 0; k < numkeys; k++)
	{
		uint8 key = zxkeys[k];
		if (key & c)
		{
			newkeys.set_key(csh);
			ownmeta = true;
			key -= c;
		}
		if (key & s)
		{
			newkeys.set_key(ssh);
			ownmeta = true;
			key -= s;
		}
		if (key == NOKEY) continue;
		newkeys.set_key(key);
		if ((key & 7) >= 5) ownmeta = true;
	}

	if (modifiers & ShiftKeyMask && !ownmeta) newkeys.set_key(csh);
	if (modifiers & ControlKeyMask) newkeys.set_key(ssh);
	if (modifiers & AltKeyMask) newkeys.set_key(csh);

	if (stickykeys)
	{
		if (stickykeys & symbolshiftsticky) newkeys.set_key(ssh);
		if (stickykeys & symbolshift2sticky) newkeys.set_key(ssh2);
		if (stickykeys & capsshiftsticky) newkeys.set_key(csh);
		if (stickykeys & capsshift2sticky) newkeys.set_key(csh2);
	}

	// update visual keymap and ula matrix:
	keymap = newkeys;
	convert_to_matrix(newkeys);
	add_ghost_keys(newkeys);
	machine->ula->keymap = newkeys;
}

void Keyboard::specciKeyDown(uint8 zxkey)
{
	// mark Specci Key Down
	// called from Keyboard Inspector for mouse click on virtual key

	xlogIn("Keyboard:specciKeyDown");
	assert(numkeys < NELEM(zxkeys));

	if (zxkey == NOKEY) return;

	zxkeys[numkeys]	  = zxkey;
	oskeys[numkeys++] = 0xff; // 0xff -> virtual, not on real kbd
	update_keymap();
}

void Keyboard::specciKeyUp(uint8 zxkey)
{
	// mark Specci Key Up
	// called from Keyboard Inspector for mouse release on virtual key

	xlogIn("Keyboard:specciKeyUp");

	assert(zxkey != NOKEY);

	for (uint i = 0; i < numkeys; i++)
	{
		if (oskeys[i] != 0xff || zxkeys[i] != zxkey) continue;
		zxkeys[i] = zxkeys[--numkeys];
		oskeys[i] = oskeys[numkeys];
		break;
	}

	if (stickykeys) // any key sticky down ?
	{
		stickykeys = 0; // => release any sticky keys
	}
	else // no key sticky down
	{	 // => shift key going up becomes sticky
		if (zxkey == csh) stickykeys = capsshiftsticky;
		if (zxkey == ssh) stickykeys = symbolshiftsticky;
		if (zxkey == csh2) stickykeys = capsshift2sticky;
		if (zxkey == ssh2) stickykeys = symbolshift2sticky;
	}

	update_keymap();
}

void Keyboard::realKeyDown(uint16 unicode, uint8 oskeycode, KbdModifiers modifiers)
{
	// key down notification
	// called by MachineController
	//
	// unicode		resulting printable character or 0 if modifier
	// keycode		OSX key code
	// modifiers	mask of all pressed modifiers

	if (mode == kbdbtzxkbd)
	{
		keyBtZxKbd(unicode, oskeycode, modifiers);
		return;
	}

	xlogIn("Keyboard.keyPressed: 0x%04x, 0x%02x, 0x%08x", unicode, oskeycode, modifiers);

	this->modifiers = modifiers;

	if (unicode) // not a modifier
	{
		uint8 lkey = unicode < sizeof(ascii2zxkey_map) ? ascii2zxkey_map[unicode] : uint8(NOKEY);	  // logical
		uint8 pkey = oskeycode < sizeof(oskey2zxkey_map) ? oskey2zxkey_map[oskeycode] : uint8(NOKEY); // physical

		uint8 zxkey = mode == kbdgame && pkey != NOKEY ? pkey : lkey;

		// uint8 zxkey = mode==kbdgame || (modifiers&(ControlKeyMask|AltKeyMask))
		//				? (pkey!=NOKEY ? pkey : lkey)
		//				: (lkey!=NOKEY ? lkey : pkey);
		if (zxkey == NOKEY) goto x;

		// Weil Keyup- und Keydown-Events manchmal nicht den gleichen Zeichencode haben
		// und Keyup-Events manchmal ganz fehlen, dürfen keine Dubletten eingetragen werden,
		// um eine deshalb hängende Taste mittels nochmaligem Drücken lösen zu können.
		for (uint i = 0; i < numkeys; i++)
		{
			if (oskeys[i] == oskeycode) goto x;
		}

		// Taste in Liste eintragen.
		assert(numkeys < NELEM(zxkeys));
		zxkeys[numkeys]	  = zxkey;
		oskeys[numkeys++] = oskeycode;
	}

x:
	update_keymap();
}

void Keyboard::keyBtZxKbd(uint16 unicode, uint8 oskeycode, KbdModifiers modifiers)
{
	xlogline(
		"key down event: '%s', unicode=%u, modifiers=$%04x on BtZxKbd",
		unicode >= ' ' && unicode <= '~' ? charstr(unicode) : "non-ascii", uint(unicode), uint(modifiers));

	uint8 pkey = unicode < sizeof(btzxkbd2zxkey_zxsp) ? btzxkbd2zxkey_zxsp[unicode] : uint8(NOKEY); // physical

	if (modifiers) { xlogline("*** modifiers set! ***"); }

	if (pkey == NOKEY)
	{
		xlogline("*** key not in documented keymap! ***");
		goto x; // TODO: switch to BASIC or normal Game mode?
	}

	if (pkey & up)
	{
		pkey -= up;

		// Taste aus Liste der gedrückten Tasten austragen:
		for (uint i = 0; i < numkeys; i++)
		{
			if (zxkeys[i] != pkey) continue;
			zxkeys[i] = zxkeys[--numkeys];
			oskeys[i] = oskeys[numkeys];
			break;
		}
		stickykeys = 0x0000; // key up => release sticky keys
	}
	else
	{
		// Taste in Liste eintragen:
		// Vorsichtshalber keine Dubletten eingetragen:
		for (uint i = 0; i < numkeys; i++)
		{
			if (zxkeys[i] == pkey) goto x;
		}
		assert(numkeys < NELEM(zxkeys));
		zxkeys[numkeys]	  = pkey;
		oskeys[numkeys++] = oskeycode;
	}

x:
	update_keymap();
}

void Keyboard::realKeyUp(uint16 unicode, uint8 oskeycode, KbdModifiers modifiers)
{
	// key up notification
	// called by MachineController
	//
	// keycode		OSX-Keycode+1 oder 0 falls nur Modifier
	// modifiers	Maske aller gedrückter Modifiers

	if (mode == kbdbtzxkbd) // sends keydown events for key down and key up!
	{
		xlogline(
			"key up event: '%s', unicode=%u, modifiers=$%04x on BtZxKbd *** ERROR! ***",
			unicode >= ' ' && unicode <= '~' ? charstr(unicode) : "non-ascii", uint(unicode), uint(modifiers));
		return;
	}

	xlogIn("Keyboard:keyReleased: 0x%02x", int(oskeycode));

	this->modifiers = modifiers;

	// Taste aus Liste der gedrückten Tasten austragen:

	if (unicode) // not a modifier
		for (uint i = 0; i < numkeys; i++)
		{
			if (oskeys[i] != oskeycode) continue;
			zxkeys[i] = zxkeys[--numkeys];
			oskeys[i] = oskeys[numkeys];
			break;
		}

	stickykeys = 0x0000; // key up => release sticky keys
	update_keymap();
}
