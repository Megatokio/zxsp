// Copyright (c) 1996 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Z80 disassembler
	----------------

	30.Jan.1995	kio	Started work on disassembler stuff
	02.Jun.1995	kio	Reworked completely
	01.Jan.1999	kio	port to linux
	25.jun.2000	kio	rewort to accomodate changed Z80 inheritance
	2004-12-15 kio	reworked
*/


#define SAFE 3
#define LOG	 1

#include "Z80.h"


// ----	opcode definitions ------------------------------------------------------------------

enum {
	NIX,
	NOP,
	LD,
	INC,
	DEC,
	RLCA,
	EX,
	ADD,
	RRCA,
	DJNZ,
	RLA,
	JR,
	RRA,
	DAA,
	CPL,
	HALT,
	SCF,
	CCF,
	RLC,
	RRC,
	RL,
	RR,
	SLA,
	SRA,
	SLL,
	SRL,
	IN,
	OUT,
	SBC,
	NEG,
	RETN,
	IM,
	ADC,
	RETI,
	RRD,
	RLD,
	SUB,
	AND,
	XOR,
	OR,
	CP,
	BIT,
	RES,
	SET,
	LDI,
	CPI,
	INI,
	OUTI,
	LDD,
	CPD,
	IND,
	OUTD,
	LDIR,
	CPIR,
	INIR,
	OTIR,
	LDDR,
	CPDR,
	INDR,
	OTDR,
	RET,
	POP,
	JP,
	CALL,
	PUSH,
	RST,
	PFX,
	EXX,
	DI,
	EI,
	BC,
	DE,
	HL,
	IX,
	IY,
	SP,
	AF,
	AF2,
	B,
	C,
	D,
	E,
	H,
	L,
	XHL,
	A, // <- KEEP THIS ORDER!
	XBC,
	XDE,
	R,
	I,
	XC,
	XSP,
	PC,
	F,
	N0,
	N1,
	N2,
	N3,
	N4,
	N5,
	N6,
	N7,
	N00,
	N08,
	N10,
	N18,
	N20,
	N28,
	N30,
	N38,
	Z,
	NZ,
	NC,
	PO,
	PE,
	M,
	P,
	N,
	NN,
	XNN,
	XN,
	DIS,
	CB,
	ED,
	XH,
	XL,
	YH,
	YL,
	XIX,
	XIY
};

static const char* word[] = {
	"",		"nop",	  "ld",	  "inc",  "dec",  "rlca", "ex",	  "add",  "rrca", "djnz", "rla", "jr",		 "rra",
	"daa",	"cpl",	  "halt", "scf",  "ccf",  "rlc",  "rrc",  "rl",	  "rr",	  "sla",  "sra", "sll",		 "srl",
	"in",	"out",	  "sbc",  "neg",  "retn", "im",	  "adc",  "reti", "rrd",  "rld",  "sub", "and",		 "xor",
	"or",	"cp",	  "bit",  "res",  "set",  "ldi",  "cpi",  "ini",  "outi", "ldd",  "cpd", "ind",		 "outd",
	"ldir", "cpir",	  "inir", "otir", "lddr", "cpdr", "indr", "otdr", "ret",  "pop",  "jp",	 "call",	 "push",
	"rst",	"prefix", "exx",  "di",	  "ei",	  "bc",	  "de",	  "hl",	  "ix",	  "iy",	  "sp",	 "af",		 "af'",
	"b",	"c",	  "d",	  "e",	  "h",	  "l",	  "(hl)", "a",	  "(bc)", "(de)", "r",	 "i",		 "(c)",
	"(sp)", "pc",	  "f",	  "0",	  "1",	  "2",	  "3",	  "4",	  "5",	  "6",	  "7",	 "$00",		 "$08",
	"$10",	"$18",	  "$20",  "$28",  "$30",  "$38",  "z",	  "nz",	  "nc",	  "po",	  "pe",	 "m",		 "p",
	"N",	"NN",	  "(NN)", "(N)",  "dis",  "cb",	  "ed",	  "xh",	  "xl",	  "yh",	  "yl",	 "(ix+dis)", "(iy+dis)"};

static int8 cmd_00[64][3] = {{NOP, 0, 0},	{LD, BC, NN},  {LD, XBC, A},  {INC, BC, 0},	 {INC, B, 0},	 {DEC, B, 0},
							 {LD, B, N},	{RLCA, 0, 0},  {EX, AF, AF2}, {ADD, HL, BC}, {LD, A, XBC},	 {DEC, BC, 0},
							 {INC, C, 0},	{DEC, C, 0},   {LD, C, N},	  {RRCA, 0, 0},	 {DJNZ, DIS, 0}, {LD, DE, NN},
							 {LD, XDE, A},	{INC, DE, 0},  {INC, D, 0},	  {DEC, D, 0},	 {LD, D, N},	 {RLA, 0, 0},
							 {JR, DIS, 0},	{ADD, HL, DE}, {LD, A, XDE},  {DEC, DE, 0},	 {INC, E, 0},	 {DEC, E, 0},
							 {LD, E, N},	{RRA, 0, 0},   {JR, NZ, DIS}, {LD, HL, NN},	 {LD, XNN, HL},	 {INC, HL, 0},
							 {INC, H, 0},	{DEC, H, 0},   {LD, H, N},	  {DAA, 0, 0},	 {JR, Z, DIS},	 {ADD, HL, HL},
							 {LD, HL, XNN}, {DEC, HL, 0},  {INC, L, 0},	  {DEC, L, 0},	 {LD, L, N},	 {CPL, 0, 0},
							 {JR, NC, DIS}, {LD, SP, NN},  {LD, XNN, A},  {INC, SP, 0},	 {INC, XHL, 0},	 {DEC, XHL, 0},
							 {LD, XHL, N},	{SCF, 0, 0},   {JR, C, DIS},  {ADD, HL, SP}, {LD, A, XNN},	 {DEC, SP, 0},
							 {INC, A, 0},	{DEC, A, 0},   {LD, A, N},	  {CCF, 0, 0}};

static int8
	cmd_C0[64][3] = {{RET, NZ, 0},	 {POP, BC, 0},	 {JP, NZ, NN},	{JP, NN, 0},   {CALL, NZ, NN},
					 {PUSH, BC, 0},	 {ADD, A, N},	 {RST, N00, 0}, {RET, Z, 0},   {RET, 0, 0},
					 {JP, Z, NN},	 {PFX, CB, 0},	 {CALL, Z, NN}, {CALL, NN, 0}, {ADC, A, N},
					 {RST, N08, 0},	 {RET, NC, 0},	 {POP, DE, 0},	{JP, NC, NN},  {OUT, XN, A},
					 {CALL, NC, NN}, {PUSH, DE, 0},	 {SUB, A, N},	{RST, N10, 0}, {RET, C, 0},
					 {EXX, 0, 0},	 {JP, C, NN},	 {IN, A, XN},	{CALL, C, NN}, {PFX, IX, 0},
					 {SBC, A, N},	 {RST, N18, 0},	 {RET, PO, 0},	{POP, HL, 0},  {JP, PO, NN},
					 {EX, HL, XSP},	 {CALL, PO, NN}, {PUSH, HL, 0}, {AND, A, N},   {RST, N20, 0},
					 {RET, PE, 0},	 {JP, HL, 0},	 {JP, PE, NN},	{EX, DE, HL},  {CALL, PE, NN},
					 {PFX, ED, 0},	 {XOR, A, N},	 {RST, N28, 0}, // ld pc,hl statt jp(hl) da: (hl) wird zu (ix+dis)
					 {RET, P, 0},	 {POP, AF, 0},	 {JP, P, NN},	{DI, 0, 0},	   {CALL, P, NN},
					 {PUSH, AF, 0},	 {OR, A, N},	 {RST, N30, 0}, {RET, M, 0},   {LD, SP, HL},
					 {JP, M, NN},	 {EI, 0, 0},	 {CALL, M, NN}, {PFX, IY, 0},  {CP, A, N},
					 {RST, N38, 0}};

static int8 cmd_ED40[64][3] = {
	{IN, B, XC}, {OUT, XC, B},	{SBC, HL, BC}, {LD, XNN, BC}, {NEG, 0, 0}, {RETN, 0, 0}, {IM, N0, 0}, {LD, I, A},
	{IN, C, XC}, {OUT, XC, C},	{ADC, HL, BC}, {LD, BC, XNN}, {NEG, 0, 0}, {RETI, 0, 0}, {IM, N0, 0}, {LD, R, A},
	{IN, D, XC}, {OUT, XC, D},	{SBC, HL, DE}, {LD, XNN, DE}, {NEG, 0, 0}, {RETN, 0, 0}, {IM, N1, 0}, {LD, A, I},
	{IN, E, XC}, {OUT, XC, E},	{ADC, HL, DE}, {LD, DE, XNN}, {NEG, 0, 0}, {RETI, 0, 0}, {IM, N2, 0}, {LD, A, R},
	{IN, H, XC}, {OUT, XC, H},	{SBC, HL, HL}, {LD, XNN, HL}, {NEG, 0, 0}, {RETN, 0, 0}, {IM, N0, 0}, {RRD, 0, 0},
	{IN, L, XC}, {OUT, XC, L},	{ADC, HL, HL}, {LD, HL, XNN}, {NEG, 0, 0}, {RETI, 0, 0}, {IM, N0, 0}, {RLD, 0, 0},
	{IN, F, XC}, {OUT, XC, N0}, {SBC, HL, SP}, {LD, XNN, SP}, {NEG, 0, 0}, {RETN, 0, 0}, {IM, N1, 0}, {NOP, 0, 0},
	{IN, A, XC}, {OUT, XC, A},	{ADC, HL, SP}, {LD, SP, XNN}, {NEG, 0, 0}, {RETI, 0, 0}, {IM, N2, 0}, {NOP, 0, 0}};

static int8 cmd_halt[] = {HALT, 0, 0};
static int8 cmd_nop[]  = {NOP, 0, 0};
static int8 c_ari[]	   = {ADD, ADC, SUB, SBC, AND, XOR, OR, CP};
static int8 c_blk[]	   = {LDI, CPI, INI,  OUTI, 0,	  0,	0, 0, LDD, CPD, IND,  OUTD, 0,	  0,
						  0,   0,	LDIR, CPIR, INIR, OTIR, 0, 0, 0,   0,	LDDR, CPDR, INDR, OTDR};
static int8 c_sh[]	   = {RLC, RRC, RL, RR, SLA, SRA, SLL, SRL};


// ============================================================================================


// ----	return m[3] mnenonic descriptor for normal instructions
//		note: for immediate use only, returned result becomes invalid with next call!
static int8* mnemo(uchar op)
{
	static int8 cl[3] = {LD, A, A};
	static int8 ca[3] = {ADD, A, A};

	switch (op >> 6)
	{
	case 0: return cmd_00[op];
	case 1:
		if (op == 0x76) return cmd_halt;
		cl[1] = B + ((op >> 3) & 0x07);
		cl[2] = B + (op & 0x07);
		return cl;
	case 2:
		ca[0] = c_ari[(op >> 3) & 0x07];
		ca[2] = B + (op & 0x07);
		return ca;
	// case 3:
	default: return cmd_C0[op & 0x3f];
	}
}


// ----	return m[3] mnenonic descriptor for CB instructions
//		note: for immediate use only!
static int8* mnemoCB(uint8 op)
{
	static int8 cmd[3];

	switch (op >> 6)
	{
	case 0:
		cmd[0] = c_sh[(op >> 3) & 0x07];
		cmd[1] = B + (op & 0x07);
		cmd[2] = 0;
		return cmd;
	case 1: cmd[0] = BIT; break;
	case 2: cmd[0] = RES; break;
	case 3: cmd[0] = SET; break;
	}
	cmd[1] = N0 + ((op >> 3) & 0x07);
	cmd[2] = B + (op & 0x07);
	return cmd;
}


// ----	return m[3] mnenonic descriptor for IXCB instructions
//		note: for immediate use only!
static int8* mnemoIXCB(uint8 op)
{
	int8* c = mnemoCB(op);
	if (c[1] == XHL) c[1] = XIX; // this is only allowed, because mnemo() doesn't
	if (c[2] == XHL) c[2] = XIX; // retrieve a pointer but creates mnemo descr ad hoc
	return c;
}


// ----	return m[3] mnenonic descriptor for IYCB instructions
//		note: for immediate use only!
static int8* mnemoIYCB(uint8 op)
{
	int8* c = mnemoCB(op);
	if (c[1] == XHL) c[1] = XIY; // this is only allowed, because mnemo() doesn't
	if (c[2] == XHL) c[2] = XIY; // retrieve a pointer but creates mnemo descr ad hoc
	return c;
}


// ----	return m[3] mnenonic descriptor for ED instructions
//		note: for immediate use only!
static int8* mnemoED(uint8 op)
{
	static int8 cmd[3] = {0, 0, 0};

	if (op < 0x40) return cmd_nop;

	if (op >= 0x080)
	{
		if ((op & 0xE4) != 0xA0) return cmd_nop;
		cmd[0] = c_blk[op & 0x1B];
		return cmd;
	};

	return cmd_ED40[op - 0x40];
}


// ----	return m[3] mnenonic descriptor for IX instructions
//		note: for immediate use only!
static int8* mnemoIX(uint8 op)
{
	static int8 cmd[3];

	memcpy(cmd, mnemo(op), 3);

	if (cmd[1] == XHL)
	{
		cmd[1] = XIX;
		return cmd;
	}
	if (cmd[2] == XHL)
	{
		cmd[2] = XIX;
		return cmd;
	}
	if (cmd[1] == HL)
	{
		cmd[1] = IX;
		return cmd;
	}
	if (cmd[2] == HL)
	{
		cmd[2] = IX;
		return cmd;
	}
	if (cmd[1] == H) cmd[1] = XH;
	if (cmd[1] == L) cmd[1] = XL;
	if (cmd[2] == H) cmd[2] = XH;
	if (cmd[2] == L) cmd[2] = XL;
	return cmd;
}


//----	return m[3] mnenonic descriptor for IY instructions
//		note: for immediate use only!
static int8* mnemoIY(uint8 op)
{
	static int8 cmd[3];

	memcpy(cmd, mnemo(op), 3);

	if (cmd[1] == XHL)
	{
		cmd[1] = XIY;
		return cmd;
	}
	if (cmd[2] == XHL)
	{
		cmd[2] = XIY;
		return cmd;
	}
	if (cmd[1] == HL)
	{
		cmd[1] = IY;
		return cmd;
	}
	if (cmd[2] == HL)
	{
		cmd[2] = IY;
		return cmd;
	}
	if (cmd[1] == H) cmd[1] = YH;
	if (cmd[1] == L) cmd[1] = YL;
	if (cmd[2] == H) cmd[2] = YH;
	if (cmd[2] == L) cmd[2] = YL;
	return cmd;
}


// ----	return mnenonic with symbolic arguments for instructions
// 		op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))
//
static char* Z80OpMnemo(uint8 op1, uint8 op2, uint8 op4)
{
	static char s[20];
	int8*		m;

	switch (op1)
	{
	case 0xCB: m = mnemoCB(op2); break;
	case 0xED: m = mnemoED(op2); break;
	case 0xDD: m = op2 == 0xCB ? mnemoIXCB(op4) : mnemoIX(op2); break;
	case 0xFD: m = op2 == 0xCB ? mnemoIYCB(op4) : mnemoIY(op2); break;
	default: m = mnemo(op1); break;
	}

	strcpy(s, word[(int)m[0]]);
	if (m[1])
	{
		strcat(s, "    ");
		s[5] = 0;
		strcat(s, word[(int)m[1]]);
		if (m[2])
		{
			strcat(s, ",");
			strcat(s, word[(int)m[2]]);
		};
	}
	return s;
}


char* Z80::getOpcodeMnemo(uint16 ip) const { return Z80OpMnemo(peek(ip), peek(ip + 1), peek(ip + 3)); }


// ================================================================================


// ----	Calculate length of instruction ------------------------------- 30.jun.95 KIO !
//		op2 is only used if op1 is a prefix instruction
//		IX/IY before IX/IY/ED have no effect and are reported as length 1
int Z80OpcodeLength(uint8 op1, uint8 op2)
{
	static int8 len0[] = "1311112111111121231111212111112123311121213111212331112121311121"; // 0x00 - 0x3F
	static int8 len3[] =
		"1133312111303321113231211132302111313121113130211131312111313021"; // 0xC0 - 0xFF; prefixes are 0

	switch (op1 >> 6)
	{
	case 0: return len0[op1] - '0'; // 0x00 - 0x3F:	various length
	case 1:							// 0x40 - 0x7F: ld r,r: all 1
	case 2: return 1;				// 0x80 - 0xBF:	arithmetics/logics op a,r: all 1
	}

	switch (op1) // test for prefix
	{
	case 0xcb: return 2;
	case 0xed:
		if (/* op2<0x40 || op2>=0x80 || ((op2&7)!=3) */ (op2 & 0xc7) != 0x43) return 2;
		else return 4;
	case 0xdd:
	case 0xfd:
		switch (op2 >> 6)
		{
		case 0:
			return len0[op2] - '0' + 1 + (op2 >= 0x34 && op2 <= 0x36); // inc(hl); dec(hl); ld(hl),N: add displacement
		case 1:
		case 2:
			if (((op2 & 0x07) == 6) == ((op2 & 0x0F8) == 0x70)) return 2;
			else return 3;
		}
		if (op2 == 0xcb) return 4;
		return len3[op2 & 0x3F] - '0' +
			   1; // note: entries for prefixes are 0 giving a total of 1, just to skip the useless prefix
	}

	return len3[op1 & 0x3F] - '0'; // 0xC0 - 0xFF:	no prefix:	various length
}


int Z80::getOpcodeLength(uint16 ip) const { return Z80OpcodeLength(peek(ip), peek(ip + 1)); }


// ================================================================================


// ----	get legal state of CB instruction --------------------------------------
//		all instructions legal except: sll is illegal
inline int IllegalCB(uint8 op) { return op >= 0x30 && op < 0x38 ? Z80::illegal : Z80::legal; }


// ---- get legal state of ED instruction --------------------------------------
//		0x00-0x3F and 0x80-0xFF weird except block instructions
//		0x40-0x7F legal or weird
//		in f,(c) is legal; out (c),0 is weird
inline int IllegalED(uint8 op)
{
	const char* il = "1111111111110101111100111111001111110001111100011011000011110000";

	if ((op >> 6) == 1) return il[op - 0x40] - '0' ? Z80::weird : Z80::legal;
	return *mnemoED(op) == NOP ? Z80::weird : Z80::legal;
}


// ---- get legal state of IX/IY instruction --------------------------------------
//		all illegal instructions, which use XH or XL are illegal
//		all illegal instructions, which don't use XH or XL are weird
//		prefixes are legal
inline int IllegalXY(uint8 op)
{
	int8* c;

	c = mnemo(op);

	if (c[0] == PFX || c[1] == XHL || c[2] == XHL) return Z80::legal;
	if (c[1] == H || c[1] == L || c[2] == H || c[2] == L) return Z80::illegal;
	return Z80::weird;
}


// ---- get legal state of IXCB/IYCB instruction ----------------------------------
//		all instructions which do not use IX are weird
//		instructions using IX are legal except: sll is illegal
inline int IllegalXYCB(uint8 op)
{
	if ((op & 0x07) != 6) return Z80::weird;
	return op >= 0x30 && op < 0x38 ? Z80::illegal : Z80::legal;
}


// ----	get legal state of instruction --------------------------------------
// 		op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))
//
static int Z80OpLegal(uint8 op1, uint8 op2, uint8 op4)
{
	switch (op1)
	{
	case 0xCB: return IllegalCB(op2);
	case 0xED: return IllegalED(op2);
	case 0xDD:
	case 0xFD: return op2 == 0xCB ? IllegalXYCB(op4) : IllegalXY(op2);
	default: return Z80::legal;
	}
}


int Z80::getOpcodeLegalState(uint16 ip) const { return Z80OpLegal(peek(ip), peek(ip + 1), peek(ip + 3)); }


// ===================================================================================

#define NEXTBYTE	peek(ip++)
#define SKIPBYTE(N) ip += (N)


// ----	expand token n, reading opcode parameters via *ip++ if needed ----------
//		note: for immediate use only!
char* Z80::_xword(uint8 n, uint16& ip) const
{
	uint16		nn;
	static char s[20];

	switch (n)
	{
	case DIS:
		n  = NEXTBYTE;
		nn = ip + (char)n;
		sprintf(s, "$%04hX", nn); // branch destination
		break;
	case N: sprintf(s, "%hhi", NEXTBYTE); break;
	case NN:
		n  = NEXTBYTE;
		nn = n + 256 * NEXTBYTE;
		sprintf(s, "$%04hX", nn);
		break;
	case XNN:
		n  = NEXTBYTE;
		nn = n + 256 * NEXTBYTE;
		sprintf(s, "($%04hX)", nn);
		break;
	case XN: sprintf(s, "($%02hhX)", NEXTBYTE); break;
	case XIX: sprintf(s, "(ix%+hhi)", NEXTBYTE); break;
	case XIY: sprintf(s, "(iy%+hhi)", NEXTBYTE); break;
	default: strcpy(s, word[n]);
	}

	return s;
}


// ----	disassemble one instruction and increment ip
//		note: for immediate use only!
char* Z80::disassemble(uint16& ip) const
{
	uint8		op;
	int8*		m;
	static char s[40];
	bool		xycb = false;

	op = NEXTBYTE;

	switch (op)
	{
	case 0xcb: m = mnemoCB(NEXTBYTE); break;
	case 0xed: m = mnemoED(NEXTBYTE); break;
	case 0xdd:
		op = NEXTBYTE;
		if (op != 0xCB)
		{
			m = mnemoIX(op);
			break;
		}
		SKIPBYTE(1);
		m = mnemoIXCB(NEXTBYTE);
		SKIPBYTE(-2);
		xycb = true;
		break;
	case 0xfd:
		op = NEXTBYTE;
		if (op != 0xCB)
		{
			m = mnemoIY(op);
			break;
		}
		SKIPBYTE(1);
		m = mnemoIYCB(NEXTBYTE);
		SKIPBYTE(-2);
		xycb = true;
		break;
	default: m = mnemo(op); break;
	}

	strcpy(s, word[(int)m[0]]);

	if (m[1])
	{
		strcat(s, "    ");
		s[5] = 0;
		strcat(s, _xword(m[1], ip));
		if (m[2])
		{
			strcat(s, ",");
			strcat(s, _xword(m[2], ip));
		}
	}

	if (xycb) SKIPBYTE(1);
	return s;
}
