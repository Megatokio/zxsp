// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#define LOGLEVEL 1
#include "SP0256.h"
#include "Dsp.h"
#include "unix/FD.h"
#include <math.h>


#define PRINT_STATISTICS  0
#define DISASS_ALLOPHONES 0


// issues:

// cmd 46 'WW' sounds like a bass bang
// cmd 51 'ER1' the 'r' is somehow not there
// cmd 39 'RR2' the 'r' is somehow sounds like 'n'


// --------------------------------------------------------------
//					tables and stuff:
// --------------------------------------------------------------


/* nicht-lineare Umrechentabelle für Koeffizienten 7 bit -> 9 bit
   aus SP0250 Datenblatt
*/
static const uint16 coeff_tab[128] = {
	0,	 9,	  17,  25,	33,	 41,  49,  57,	65,	 73, // +8		37x
	81,	 89,  97,  105, 113, 121, 129, 137, 145, 153, 161, 169, 177, 185, 193, 201, 209, 217, 225, 233,
	241, 249, 257, 265, 273, 281, 289, 297, 301, 305, // +4		32x
	309, 313, 317, 321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369, 373, 377, 381, 385,
	389, 393, 397, 401, 405, 409, 413, 417, 421, 425,

	427, 429, 431, 433, 435, 437, 439, 441, 443, 445,													// +2		28x
	447, 449, 451, 453, 455, 457, 459, 461, 463, 465, 467, 469, 471, 473, 475, 477, 479, 481, 482, 483, // +1		30x
	484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503,
	504, 505, 506, 507, 508, 509, 510, 511};


/* Indexes in Koeffizienten-Liste c[]
 */
#define B0 c[0]
#define B1 c[2]
#define B2 c[4]
#define B3 c[6]
#define B4 c[8]
#define B5 c[10]
#define F0 c[1]
#define F1 c[3]
#define F2 c[5]
#define F3 c[7]
#define F4 c[9]
#define F5 c[11]


/*	Enumeration of SP0256 micro sequencer opcodes:
	opcode values are for not-bitswapped rom!
*/
enum {
	SETPAGE = 0b0000,
	SETMODE = 0b0001,
	LOAD_4	= 0b0010,
	LOAD_C	= 0b0011,
	LOAD_2	= 0b0100,
	SETMSBA = 0b0101,
	SETMSB6 = 0b0110,
	LOAD_E	= 0b0111,
	LOADALL = 0b1000,
	DELTA_9 = 0b1001,
	SETMSB5 = 0b1010,
	DELTA_D = 0b1011,
	SETMSB3 = 0b1100,
	JSR		= 0b1101,
	JMP		= 0b1110,
	PAUSE	= 0b1111
};

static const char opcode_names[64][8] = {
	"SETPAGE",
	"SETMODE",
	"LOAD_4 ",
	"LOAD_C ",
	"LOAD_2 ",
	"SETMSBA",
	"SETMSB6",
	"LOAD_E ",
	"LOADALL",
	"DELTA_9",
	"SETMSB5",
	"DELTA_D",
	"SETMSB3",
	"JSR    ",
	"JMP    ",
	"PAUSE  ",
};


/*	Allophone names for the AL2 rom:
 */
static const char al2_allophone_names[64][4] = {
	"PA1", //	10	  pause
	"PA2", //	30	  pause
	"PA3", //	50	  pause
	"PA4", //	100	  pause
	"PA5", //	200	  pause

	"OY",  //	420   oy
	"AY",  //	260   (ii)
	"EH",  //	70    e
	"KK3", //  120   c
	"PP",  //	210   p
	"JH",  //	140   j
	"NN1", //  140   n
	"IH",  //	70    i
	"TT2", //  140   (tt)
	"RR1", //  170   (rr)
	"AX",  //	70    u
	"MM",  //	180   m
	"TT1", //  100   t
	"DH1", //  290   dth
	"IY",  //	250   (ee)
	"EY",  //	280   (aa), (ay)
	"DD1", //	70    d
	"UW1", //  100   ou
	"AO",  //	100   o
	"AA",  //	100   a
	"YY2", //  180   (yy)
	"AE",  //	120   eh
	"HH1", //  130   h
	"BB1", //	80    b
	"TH",  //	180   th
	"UH",  //	100   uh
	"UW2", //  260   ouu
	"AW",  //	370   ow
	"DD2", //  160   (dd)
	"GG3", //  140   (ggg)
	"VV",  //	190   v
	"GG1", //	80    g
	"SH",  //	160   sh
	"ZH",  //	190   zh
	"RR2", //  120   r
	"FF",  //	150   f
	"KK2", //  190   ck? (gg)?
	"KK1", //  160   k
	"ZZ",  //	210   z
	"NG",  //	220   ng
	"LL",  //	110   l
	"WW",  //	180   w
	"XR",  //	360   aer
	"WH",  //	200   wh
	"YY1", //	130   y
	"CH",  //	190   ch
	"ER1", //	160   er
	"ER2", //	300   err
	"OW",  //	240   (oo), (eau)
	"DH2", //	240   ?
	"SS",  //	90	  s
	"NN2", //	190   (nn)
	"HH2", //	180   (hh)
	"OR",  //	330   or
	"AR",  //	290   ar
	"YR",  //	350   ear
	"GG2", //	40    ?
	"EL",  //	190   (ll)
	"BB2", //	50    (bb)
};


/*
// bit-swap nibble:
#define X4(N)	((((N)>>3)&1) + (((N)>>1)&2) + (((N)<<1)&4) + (((N)<<3)&8))

// bit-swap byte:
#define X8(N)	((((N)>>7)&1) + (((N)>>5)&2) + (((N)>>3)&4) + (((N)>>1)&8) + \
				 (((N)&8)<<1) + (((N)&4)<<3) + (((N)&2)<<5) + (((N)&1)<<7))


inline uint32 X32(uint32 n)
{
	n = ((n & 0xFFFF0000) >> 16) | ((n & 0x0000FFFF) << 16);
	n = ((n & 0xFF00FF00) >>  8) | ((n & 0x00FF00FF) <<  8);
	n = ((n & 0xF0F0F0F0) >>  4) | ((n & 0x0F0F0F0F) <<  4);
	n = ((n & 0xCCCCCCCC) >>  2) | ((n & 0x33333333) <<  2);
	n = ((n & 0xAAAAAAAA) >>  1) | ((n & 0x55555555) <<  1);
	return n;
}
*/


// bit-swap byte:
inline uint8 X8(uint8 n)
{
	n = ((n & 0xF0) >> 4) | ((n & 0x0F) << 4);
	n = ((n & 0xCC) >> 2) | ((n & 0x33) << 2);
	n = ((n & 0xAA) >> 1) | ((n & 0x55) << 1);
	return n;
}

// bit-swap nibble:
inline uint8 X4(uint8 n)
{
	n = ((n & 0xC) >> 2) | ((n & 0x3) << 2);
	n = ((n & 0xA) >> 1) | ((n & 0x5) << 1);
	return n;
}


// --------------------------------------------------------------
//						c'tor, d'tor
// --------------------------------------------------------------


SP0256::SP0256(cstr romfilepath, bool is_bitswapped, float rc) :
	rc(rc),											 // filter factor for RC; e.g. RC = 33kΩ * 22nF:
	ff(1.0 - exp(-(1.0 / samples_per_second) / rc)), // see comments at output_filtered()
	time_per_sample(312 / 3.12e6), volume(0), amplification(0), hifi(yes), sm_state(0), time(0.0), time_sample_end(0),
	sample(0), sample_at_c1(0), sample_at_c2(0), current_opcode(0), repeat(0), pitch(0),
	amplitude(0), c {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, pitch_incr(0),
	amplitude_incr(0), _c {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, _z {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, _shiftreg(0),
	_i(0), mode(0), page(0), pc(0), stack(0), command(0), current_command(0), stand_by(0), command_valid(0), byte(0),
	bits(0)
{
	xlogline("new SP0256: romfile = %s", romfilepath);

	IFDEBUG(for (uint i = 0; i < 8; i++) { assert(X8(1 << i) == (128 >> i)); };)

	FD fd(romfilepath);
	fd.read_data(rom, 2048);

	// make non-reversed rom:
	if (is_bitswapped)
		for (uint i = 0; i < 2048; i++)
		{
			uint8 b = rom[i];
			rom[i]	= X8(b);
		}

	if (DISASS_ALLOPHONES) disassAllophones();
}


SP0256::~SP0256()
{
	xlogIn("~SP0256");

	if (PRINT_STATISTICS)
	{
		logline("Volume info for allophones");
		for (uint i = 5; i < 64; i++)
		{
			Stats& s = allophone_stats[i];
			logline(
				"  %s%s: max. %.4f - c1: %.4f - c2: %.4f",
				al2_allophone_names[i],
				al2_allophone_names[i][2] ? "" : " ",
				s.max_sample,
				s.max_sample_at_c1,
				s.max_sample_at_c2);
		}
		logline("Volume info for micro sequencer opcodes");
		for (uint i = 0; i < 16; i++)
		{
			Stats& s = opcode_stats[i];
			logline(
				"  %s: max. %.4f - c1: %.4f - c2: %.4f",
				opcode_names[i],
				s.max_sample,
				s.max_sample_at_c1,
				s.max_sample_at_c2);
		}
	}
}


void SP0256::set_clock(Frequency xtal) { time_per_sample = 312.0 / xtal; }

void SP0256::set_volume(Sample volume)
{
	this->volume		= volume;
	this->amplification = volume * rc * 14; // TODO: linearer Zusammenhang eigentlich nur bei einfachem RC-Glied?!
}


// --------------------------------------------------------------
//						public methods
// --------------------------------------------------------------

void SP0256::powerOn(Sample volume, Frequency xtal)
{
	xlogIn("SP0256::init");

	sm_state = 0;	// restart state machine
	time	 = 0.0; // up to which time we ran
	set_clock(xtal);
	set_volume(volume);
}

void SP0256::reset(Time t, Sample volume, Frequency xtal)
{
	xlogIn("SP0256::reset");

	run_statemachine(t);
	sm_state = 0;
	set_clock(xtal);
	set_volume(volume);
}

void SP0256::setClock(Time t, Frequency xtal)
{
	xxlogIn("SP0256::setClock");

	run_statemachine(t);
	set_clock(xtal);
}

void SP0256::setVolume(Time t, Sample volume)
{
	xxlogIn("SP0256::setVolume");

	run_statemachine(t);
	set_volume(volume);
}


/*	write a command into the SP0256 command register
	only command 0 .. 63 are valid for the AL2 rom!
*/
void SP0256::writeCommand(Time t, uint cmd)
{
	if (cmd) // µspeech sendet dauernd cmd = 0 = PA1 => nicht loggen
	{
		if (cmd > 63)
			xlogIn("SP0256::writeCommand %u = illegal entry address!", cmd);
		else
			xlogIn("SP0256::writeCommand %u = %s", cmd, al2_allophone_names[cmd]);
	}

	run_statemachine(t);

	if (command_valid) xlogline("SP0256: new command overwrote pending command");
	if (cmd && stand_by) xxlogline("SP0256: starting new command from stand-by");

	command		  = cmd;
	command_valid = yes;
	stand_by	  = no;
}


/*	test whether the SP0256 is now in stand-by (not speaking)
	note: the Currah µSpeech couldn't poll this
*/
bool SP0256::isStandby(Time t)
{
	run_statemachine(t);
	xxlogline("SP0256::isStandby = %s", stand_by ? "yes" : "no");
	return stand_by;
}


/*	test whether the SP0256 accepts a new command
	as long as stand_by is not active, the SP0256 is still speaking the previous command
*/
bool SP0256::isBusy(Time t)
{
	run_statemachine(t);
	xxlogline("SP0256::isBusy = %s", command_valid ? "yes" : "no");
	return command_valid;
}


/*	play sound up to Time t and then rewind the time base by t
	note: t may be, in rare cases, slightly less than this.time!
	returns stand_by state
*/
bool SP0256::audioBufferEnd(Time t)
{
	xxlogIn("SP0256::audioBufferEnd");
	run_statemachine(t);
	time -= t;
	time_sample_end -= t;
	return stand_by;
}


// --------------------------------------------------------------
//				helper: read bits from serial rom
// --------------------------------------------------------------


/*	read next 8 bits from non-reversed rom
	new bits come in from the left side
*/
uint8 SP0256::next8()
{
	if (bits < 8)
		byte += rom[pc++ & 0x7ff] << bits;
	else
		bits -= 8;

	uint rval = byte;
	byte	  = byte >> 8;
	return rval;
}

/*	read next N bits from rom
	new bits come in from the left side
	return value: bits are left-aligned
*/
uint8 SP0256::nextL(uint n)
{
	if (bits < n)
	{
		byte += rom[pc++ & 0x7ff] << bits;
		bits += 8;
	}
	bits -= n;

	uint rval = byte << (8 - n);
	byte	  = byte >> n;
	return rval;
}

/* signed variant:
 */
inline int8 SP0256::nextSL(uint n) { return nextL(n); }

/*	read next N bits from rom
	new bits come in from the left side
	return value: bits are right-aligned
*/
inline uint8 SP0256::nextR(uint n) { return nextL(n) >> (8 - n); }
inline int8	 SP0256::nextSR(uint n) { return nextSL(n) >> (8 - n); }


// --------------------------------------------------------------
//					handle micro sequencer opcode
// --------------------------------------------------------------

/*	Micro sequencer:

	uint2 mode
		Controls the format of data which follows various instructions.
		In some cases, it also controls whether certain filter coefficients are zeroed or left unmodified.
		The exact meaning of MODE varies by instruction.
		MODE is sticky, meaning that once it is set, it retains its value until it is explicitly changed
		by Opcode 1000 (SETMODE) or the sequencer halts.

	uint2 repeat_prefix
		The parameter load instructions can provide a four bit repeat value to the filter core.
		This register optionally extends that four bit value by providing two more significant bits in the 2 MSBs.
		By setting the repeat prefix with Opcode 1000 (SETMODE), the program can specify repeat values up to $3F (63).
		This register is not sticky.

	uint4 page
		The PAGE register acts as a prefix, providing the upper 4 address bits (15…12) for JMP and JSR instructions.
		The PAGE register can hold any binary value from 0001 to 1111, and is set by the SETPAGE instruction.
		It is not possible to load it with 0000.
		It powers up to the value 0001, and it retains its value across JMP/JSR instructions and stand-by.

	uint16 pc
		This is the program counter. This counter tracks the address of the byte that is currently being processed.
		A copy of the program counter is kept in every Speech ROM that is attached to the SP0256,
		so that the program counter is only broadcast on JMP or JSR.

	uint16 stack
		This is where the program counter is saved when performing a JSR.
		The STACK has room for exactly one address, so nested subroutines are not possible.
		It holds the address of the byte following the JSR instruction.

	uint8 command
		This holds address of the most recent command from the host CPU.
		Addresses are loaded into this register via external pins and the ALD control line.
		When the microsequencer is halted (or is about to halt), it watches for an address in this register.
		When a new command address is available, it copies these bits to bits 1 through 8 of the program counter,
		bit 12 is set to 1 and all other bits are cleared to 0, so that code executes out of page $1.


	Key for opcode formats below

	Bits come out from the serial rom LSB first.
	Note: the original rom image was bit-swapped, but we use the unswapped version!
	Most bit fields, except those which specify branch targets, are unswapped, meaning the left-most bit is the MSB.
	Bit fields narrower than 8 bits are MSB justified unless specified otherwise, meaning that the least significant
	bits are the ones that are missing. These LSBs are filled with zeros.
	When updating filter coefficients with a delta-update, the microsequencer performs plain 2s-complement arithmetic
	on the 8-bit value in the coefficient register file. No attention is paid to the format of the register.
	kio TODO: also true for the once-per-pitch update?

	Field		Description
	AAAAAAAA 	Amplitude bits. The 3 msbits are the exponent, the 5 lsbits are the mantissa.
				the amplitude is decoded 8 bit -> 12 bit at the time when it is loaded into the pulse generator.
	PPPPPPPP 	Pitch period. When set to 0, the impulse switches to random noise.
				For timing purposes, noise and silence have an effective period equivalent to period==64.
	SBBBBBBB 	B coefficient data. The 'S' is the sign bit, if present. If there is no 'S' on a given field,
				the sign is assumed to be 0. (which means NEGATIVE - kio)
				coefficients are expanded from sign+7 bit to sign+9 bit via a rom table at time of calculation.
	SFFFFFFF 	F coefficient data.
	RRRR		Repeat bits. On Opcode SETMODE, the 2 repeat bits go to the two MSBs of the repeat count
				for the next instruction. On all other instructions, the repeat bits go to the 4 LSBs of the
				repeat count for the current instruction.
	MM			Mode bits. These are set by Opcode SETMODE. They specify the data format for following opcodes.
	LLLLLLLL 	Byte address for a branch target. Branch targets are 16 bits long.
				The JMP/JSR instruction provides the lower 12 bits, and the PAGE register provides the upper 4 bits.
				The branch target bits (LLLL in SETPAGE, LLLL and LLLLLLLL in JMP and JSR) are bit-swapped.
	aaaaa		Amplitude delta. (unsigned) note: applied to the 8-bit encoded amplitude!
	ppppp		Pitch delta. (unsigned)
	saaa		Amplitude delta. (2s complement) note: applied to the 8-bit encoded amplitude!
	sppp		Pitch delta. (2s complement)
	sbbb sfff 	Filter coefficient deltas. (2s complement) note: applied to the 8-bit encoded coefficients!
*/


/*	RTS/SETPAGE – Return OR set the PAGE register
	0000 LLLL >>>
*/
void SP0256::cmdSetPage(uint llll)
{
	/*	SETPAGE
		• When LLLL is non-zero, this instruction sets the PAGE register to the value in LLLL.
		• LLLL is bit-swapped. (kio 2015-04-20 checked with rom disass)
		• The 4-bit PAGE register determines which 4K page the next JMPs or JSRs will go to.
		  Note that address loads via ALD ignore PAGE, and set the four MSBs to $1000. They do not modify
		  the PAGE register, so subsequent JMP/JSR instructions will jump relative to the current value in PAGE.
		• The PAGE register retains its setting until the next SETPAGE is encountered.
		• Valid values for PAGE are in the range $1..$F.
		• The RESROM starts at address $1000, and no code exists below that address.
		  Therefore, the microsequencer can address speech data over the address range $1000 through $FFFF, for a total
		  of 60K of speech data. (Up to 64K may be possible by wrapping around, but that's not yet verified.)

		RTS
		• When LLLL is zero, this opcode causes the microsequencer to pop the PC stack into the PC, and resume execution
		  there. The contents of the stack are replaced with $0000 (or some other flag which represents an empty stack).
		• If the address that was popped was itself $0000 (eg. an empty stack), execution halts, pending a new address
		  write via ALD. (Of course, if an address was previously written via ALD and is pending, control transfers to
		  that address immediately.)
	*/
	if (llll) // LLLL!=0 => SETPAGE
	{
		page = X4(llll) << 12;
		logline("SP0256: SETPAGE(%u)", page >> 12); // this command makes no sense with the 2kB AL2 rom
	}
	else // LLLL==0 => RTS
	{
		pc	  = stack;
		stack = byte = bits = 0; // pop address
		if (pc)
		{
			logline("SP0256: RTS"); // no JSR in AL2 rom => SETPAGE never used for RTS
			return;					// RTS
		}

		// no address on stack => next command
		if (command_valid)
		{
			xxlogline(
				"SP0256: RTS: next command = %u = %s", command, command < 64 ? al2_allophone_names[command] : "???");
			pc			  = 0x1000 + (command << 1);
			command_valid = no;
			stand_by	  = no;
			if (PRINT_STATISTICS) current_command = command;
			return;
		}

		// no next command => stand-by
		xxlogline("SP0256: RTS: stand-by");
		stand_by = yes;
	}
}


/*	JMP – Jump to Byte Address
	LLLLLLLL 1110 LLLL >>>
*/
void SP0256::cmdJmp(uint instr)
{
	/*	Performs a jump to the specified 12-bit address inside the 4K page specified by the PAGE register.
		That is, the JMP instruction jumps to the location "PAGE.LLLL.LLLLLLLL", where the upper four bits come from
		the PAGE register and the lower 12 bits come from the JMP instruction.
		At power-up, the PAGE register defaults to address $1000. (value 0b0001)
		The PAGE register may be set using opcode SETPAGE.
		note: destination address bits LLLL and LLLLLLLL are reverse ordered (kio 2015-04-20 checked with rom disass)
	*/
	pc = page + (X4(instr & 15) << 8) + X8(next8());
	xxlogline("SP0256: JMP: 0x%4u", pc);
}


/*	JSR – Jump to Subroutine
	LLLLLLLL 1101 LLLL >>>
	not used in AL2 rom
*/
void SP0256::cmdJsr(uint instr)
{
	/*	Jump to the specified 12-bit address inside the 4K page specified by the PAGE register.
		RTS pushes the byte-aligned return address onto the PC stack, which can only hold one entry.
		To return to the next instruction, use Opcode RTS.
	*/
	stack = pc + 1;
	pc	  = page + (X4(instr & 15) << 8) + X8(next8());
	xxlogline("SP0256: JSR: 0x%4u", pc);
}


/*	SETMODE – Set the Mode bits and Repeat msbits
	0001 MM RR >>>
*/
void SP0256::cmdSetMode(uint instr)
{
	/*	Prefix for parameter load opcodes.
		• Load the two RR bits into the two msbits of the 6-bit repeat register.
		  These two bits combine with the four lsbits that are provided by most parameter load opcodes
		  to provide longer repetition periods.
		  The RR bits are not sticky.
			kio: they are probably counted down to 0 by the audio filter loop
				 therefore: not only JMP and JSR, but also SETPAGE/RTS should not reset these bits.
		• The two MM bits select the data format for many of the parameter load opcodes.
		  The MM mode bits are sticky, meaning that they stay in effect until the next Opcode SETMODE instruction.
		• This opcode is known to have no effect on JMP/JSR instructions and JMP/JSR instructions have no effect on it.
	*/
	repeat = (instr & 3) << 4;
	mode   = (instr & 0xC) >> 2;
	xxlogline("SP0256: SETMODE: RR=%u, MM=%u", instr & 3, mode);
}


/*	PAUSE – Silent pause
	1111 RRRR >>>
*/
void SP0256::cmdPause()
{
	/*	Provides a silent pause of varying length.
		The length of the pause is given by the 4-bit immediate constant RRRR.
		The pause duration can be extended with the prefix opcode SETMODE.
		Notes: The pause behaves identially to a pitch with Amplitude=0 and Period=64.
		All coefficients are cleared.
	*/
	amplitude	   = 0;
	pitch		   = 64;
	amplitude_incr = 0; // not expressively stated, but otherwise pause with R>1 would not be silent
	pitch_incr	   = 0; // not expressively stated, but otherwise pause with R>1 would not be silent
	memset(c, 0, sizeof(c));

	xxlogline("SP0256: PAUSE");
}


/*	LOADALL – Load All Parameters
	[data] 1000 RRRR >>>
	not used in AL2 rom
*/
void SP0256::cmdLoadAll()
{
	/*	Load amplitude, pitch, and all coefficient pairs at full 8-bit precision.
		The pitch and amplitude deltas that are available in Mode 1x are applied every pitch period, not just once.
		Wraparound may occur. If the Pitch goes to zero, the periodic excitation switches to noise.
		TODO: also applied after the last loop?

		all modes:
		AAAAAAAA PPPPPPPP
		SBBBBBBB SFFFFFFF   (B0,F0)
		SBBBBBBB SFFFFFFF   (B1,F1)
		SBBBBBBB SFFFFFFF   (B2,F2)
		SBBBBBBB SFFFFFFF   (B3,F3)
		SBBBBBBB SFFFFFFF   (B4,F4)
		SBBBBBBB SFFFFFFF   (B5,F5)

		mode 1x:
		saaaaaaa sppppppp   (amplitude and pitch interpolation)
	*/
	amplitude = next8();
	pitch	  = next8();

	for (uint i = 0; i < 12; i++) { c[i] = next8(); }

	if (mode & 2) // 1x
	{
		amplitude_incr = next8();
		pitch_incr	   = next8();
	}
	else // 0x
	{
		amplitude_incr = 0;
		pitch_incr	   = 0;
	}

	xxlogline("SP0256: LOADALL");
}


/*	LOAD_E – Load Pitch and Amplitude
	PPPPPPPP AAAAAA 0111 RRRR >>>
*/
void SP0256::cmdLoadE()
{
	/*	Load new amplitude and pitch.
		There seem to be no data format variants for the different modes,
		although the repeat count may be extended using opcode SETMODE.

		all other registers preserved.
	*/
	amplitude = nextL(6);
	pitch	  = next8();

	xxlogline("SP0256: LOAD_E");
}


/*	LOAD_4 – Load Pitch, Amplitude and Coefficients (2 or 3 stages)
	[data] 0010 RRRR >>>
*/
void SP0256::cmdLoad4()
{
	/*	Load new amplitude and pitch parameters.
		Load new filter coefficients, setting the unspecified coefficients to 0.
		The exact combination and precision of data is determined by the mode bits as set by opcode SETMODE.
		For all modes, the sign bit for B0 has an implied value of 0.

		all modes:
		AAAAAA   PPPPPPPP

		mode x0:
		BBBB     SFFFFF     (B3,F3)
		SBBBBBB  SFFFFF     (B4,F4)

		mode x1:
		BBBBBB   SFFFFFF    (B3,F3)
		SBBBBBBB SFFFFFFF   (B4,F4)

		mode 1x:
		SBBBBBBB SFFFFFFF   (B5,F5)
	*/
	amplitude	   = nextL(6);
	pitch		   = next8();
	amplitude_incr = 0;
	pitch_incr	   = 0;

	memset(c, 0, sizeof(c)); // set the unspecified coefficients to 0

	if (mode & 1) // x1
	{
		B3 = nextL(6) >> 1;
		F3 = nextL(7);
		B4 = next8();
		F4 = next8();
	}
	else // x0
	{
		B3 = nextL(4) >> 1;
		F3 = nextL(6);
		B4 = nextL(7);
		F4 = nextL(6);
	}

	if (mode & 2) // 1x
	{
		B5 = next8();
		F5 = next8();
	}

	xxlogline("SP0256: LOAD_4");
}


/*	LOAD_C – Load Pitch, Amplitude, Coefficients (5 or 6 stages)
	LOAD_2 – Load Pitch, Amplitude, Coefficients (5 or 6 stages), and Interpolation Registers
	[data] 0011 RRRR >>>
	[data] 0100 RRRR >>>
*/
void SP0256::cmdLoad2C(uint instr)
{
	/*	Load new amplitude and pitch parameters.
		Load new filter coefficients, setting the unspecified coefficients to zero.
		The exact combination and precision of data is determined by the mode bits as set by opcode SETMODE.
		• For all Modes, the Sign bit for B0, B1, B2 and B3 has an implied value of 0.
		• LOAD_2 and LOAD_C only differ in that LOAD_2 also loads new values into the
		  Amplitude and Pitch Interpolation Registers while LOAD_C doesn't.

		all modes:
		AAAAAA   PPPPPPPP

		mode x0:
		BBB      SFFFF      (coeff pair 0)
		BBB      SFFFF      (coeff pair 1)
		BBB      SFFFF      (coeff pair 2)
		BBBB     SFFFFF     (coeff pair 3)
		SBBBBBB  SFFFFF     (coeff pair 4)

		mode x1:
		BBBBBB   SFFFFF     (coeff pair 0)
		BBBBBB   SFFFFF     (coeff pair 1)
		BBBBBB   SFFFFF     (coeff pair 2)
		BBBBBB   SFFFFFF    (coeff pair 3)
		SBBBBBBB SFFFFFFF   (coeff pair 4)

		mode 1x:
		SBBBBBBB SFFFFFFF   (coeff pair 5)

		LOAD_2 only, all modes:
		aaaaa    ppppp      (Interpolation register LSBs)
	*/
	amplitude = nextL(6);
	pitch	  = next8();

	if (mode & 1) // x1
	{
		B0 = nextL(6) >> 1;
		F0 = nextL(6);
		B1 = nextL(6) >> 1;
		F1 = nextL(6);
		B2 = nextL(6) >> 1;
		F2 = nextL(6);
		B3 = nextL(6) >> 1;
		F3 = nextL(7);
		B4 = next8();
		F4 = next8();
	}
	else // x0
	{
		B0 = nextL(3) >> 1;
		F0 = nextL(5);
		B1 = nextL(3) >> 1;
		F1 = nextL(5);
		B2 = nextL(3) >> 1;
		F2 = nextL(5);
		B3 = nextL(4) >> 1;
		F3 = nextL(6);
		B4 = nextL(7);
		F4 = nextL(6);
	}
	if (mode & 2) // 1x
	{
		B5 = next8();
		F5 = next8();
	}
	else // 0x
	{
		B5 = 0;
		F5 = 0;
	}

	if (instr == LOAD_2) // LOAD_2
	{
		amplitude_incr = nextR(5);
		pitch_incr	   = nextR(5);
	}
	else // LOAD_C
	{
		amplitude_incr = 0;
		pitch_incr	   = 0;
	}
}


/*	LOAD_2 – Load Pitch, Amplitude, Coefficients, and Interpolation Registers
	[data] 0100 RRRR >>>
*/
inline void SP0256::cmdLoad2()
{
	xxlogline("SP0256: LOAD_2");
	cmdLoad2C(LOAD_2);
}

/*	LOAD_C – Load Pitch, Amplitude, Coefficients (5 or 6 stages)
	[data] 0011 RRRR >>>
*/
inline void SP0256::cmdLoadC()
{
	xxlogline("SP0256: LOAD_C");
	cmdLoad2C(LOAD_C);
}


/*	SETMSB_6 – Load Amplitude and MSBs of 2 or 3 'F' Coefficients
	[data] 0110 RRRR >>>
	not used in AL2 rom
*/
void SP0256::cmdSetMsb6()
{
	/*	Load new amplitude. Updates the msbits of a set of filter coefficients.
		The mode bits controls the update process as noted below. Opcode SETMODE provides the mode bits.
		Notes:
		MODE x0: the 6 msbits of F3 and F4 are set, the lsbits are not modified.
		MODE x1: the 7 msbits of F3 and all 8 bits of F4 are set, the lsbit of F3 is not modified.
		MODE 1x: all 8 bits of F5 are set, and B5 is not modified.
		MODE 0x: B5 and F5 are set to zero.

		all modes:
		AAAAAA

		mode x0:
		SFFFFF              (New F3 6 MSBs)
		SFFFFF              (New F4 6 MSBs)

		mode x1:
		SFFFFFF             (New F3 7 MSBs)
		SFFFFFFF            (New F4 8 MSBs)

		mode 1x:
		SFFFFFFF            (New F5 8 MSBs)
	*/
	logline("SP0256: SETMSB_6"); // not used in AL2 rom

	amplitude = nextL(6);

	if (mode & 1) // x1
	{
		F3 = nextL(7) + (F3 & 1);
		F4 = next8();
	}
	else // x0
	{
		F3 = nextL(6) + (F3 & 3);
		F4 = nextL(6) + (F4 & 3);
	}

	if (mode & 2) // 1x
	{
		F5 = next8();
	}
	else // 0x
	{
		F5 = B5 = 0;
	}
}


/*	SETMSB_5 – Load Amplitude, Pitch, and MSBs of 3 Coefficients
	SETMSB_A – Load Amplitude,        and MSBs of 3 Coefficients
	SETMSB_3 – Load Amplitude,        and MSBs of 3 Coefficients, and Interpolation Registers
	[data] 1010 RRRR >>>
	[data] 0101 RRRR >>>
	[data] 1100 RRRR >>>
*/
void SP0256::cmdSetMsb35A(uint instr)
{
	/*	Load new amplitude and pitch parameters.
		Update the MSBs of a set of filter coefficients.
		The Mode bits control the update process as noted below. Opcode SETMODE provides the mode bits.
		Notes:
		MODE x0: Set the 5 or 6 msbits of F0, F1, and F2, the lsbits are preserved.
		MODE x1: Set the 5 or 6 msbits of F0, F1, and F2, the lsbits are preserved.
		MODE 0x: F5 and B5 are set to 0. All other coefficients are preserved.
		MODE 1x: F5 and B5 are preserved. All other coefficients are preserved.
		Opcode SETMSB_5 also sets the Pitch.
		Opcode SETMSB_3 also sets the interpolation registers.

		all opcodes, all modes:
		AAAAAA

		SETMSB_5, all modes:
		PPPPPPPP

		all opcodes, mode x0:
		SFFFF			(F0 5 MSBs)
		SFFFF			(F1 5 MSBs)
		SFFFF			(F2 5 MSBs)

		all opcodes, mode x1:
		SFFFFF			(F0 6 MSBs)
		SFFFFF			(F1 6 MSBs)
		SFFFFF			(F2 6 MSBs)

		SETMSB_3, all modes:
		aaaaa    ppppp	(incr. LSBs)
	*/
	amplitude = nextL(6);
	if (instr == SETMSB5) { pitch = next8(); }

	if (mode & 1) // x1
	{
		F0 = nextL(6) + (F0 & 3);
		F1 = nextL(6) + (F1 & 3);
		F2 = nextL(6) + (F2 & 3);
	}
	else // x0
	{
		F0 = nextL(5) + (F0 & 7);
		F1 = nextL(5) + (F1 & 7);
		F2 = nextL(5) + (F2 & 7);
	}

	if (mode & 2) // 1x
	{}
	else // 0x
	{
		F5 = B5 = 0;
	}

	if (instr == SETMSB3)
	{
		amplitude_incr = nextR(5);
		pitch_incr	   = nextR(5);
	}
}


/*	SETMSB_3 – Load Amplitude, MSBs of 3 Coefficients, and Interpolation Registers
	[data] 1100 RRRR >>>
*/
inline void SP0256::cmdSetMsb3()
{
	xxlogline("SP0256: SETMSB_3");
	cmdSetMsb35A(SETMSB3);
}

/*	SETMSB_5 – Load Amplitude, Pitch, and MSBs of 3 Coefficients
	[data] 1010 RRRR >>>
*/
inline void SP0256::cmdSetMsb5()
{
	xxlogline("SP0256: SETMSB_5");
	cmdSetMsb35A(SETMSB5);
}

/*	SETMSB_A – Load Amplitude and MSBs of 3 Coefficients
	[data] 0101 RRRR >>>
*/
inline void SP0256::cmdSetMsbA()
{
	xxlogline("SP0256: SETMSB_A");
	cmdSetMsb35A(SETMSBA);
}


/*	DELTA_9 – Delta update Amplitude, Pitch and 5 or 6 Coefficients
	[data] 1001 RRRR
*/
void SP0256::cmdDelta9()
{
	/*	Performs a delta update, adding small 2s complement numbers to a series of coefficients.
		The 2s complement updates for the various filter coefficients only update some of the MSBs,
		the LSBs are unaffected.
		The exact bits which are updated are noted below.
		Notes
		• The delta update is applied exactly once, as long as the repeat count is at least 1.
		• If the repeat count is greater than 1, the updated value is held through the repeat period,
		  but the delta update is not reapplied.
		• The delta updates are applied to the 8-bit encoded forms of the coefficients, not the 10-bit decoded forms.
		  Normal 2s complement arithmetic is performed, and no protection is provided against overflow.
		  Adding 1 to the largest value for a bit field wraps around to the smallest value for that bitfield.
		• The update to the amplitude register is a normal 2s complement update to the 8-bit register.
		  This means that any carry/borrow from the mantissa will change the value of the exponent.
		  The update doesn't know anything about the format of that register.

		all modes:
		saaa     spppp      (Amplitude 6 MSBs, Pitch LSBs.)

		mode x0:
		sbb      sff        (B0 4 MSBs, F0 5 MSBs.)
		sbb      sff        (B1 4 MSBs, F1 5 MSBs.)
		sbb      sff        (B2 4 MSBs, F2 5 MSBs.)
		sbb      sfff       (B3 5 MSBs, F3 6 MSBs.)
		sbbb     sfff       (B4 6 MSBs, F4 6 MSBs.)	// Zbiciak: 6 bits, MAME: B4 7 Bits!

		mode x1:
		sbbb     sfff       (B0 7 MSBs, F0 6 MSBs.)
		sbbb     sfff       (B1 7 MSBs, F1 6 MSBs.)
		sbbb     sfff       (B2 7 MSBs, F2 6 MSBs.)
		sbbb     sffff      (B3 7 MSBs, F3 7 MSBs.)
		sbbbb    sffff      (B4 8 MSBs, F4 8 MSBs.)

		mode 1x:
		sbbbb    sffff      (B5 8 MSBs, F5 8 MSBs.)
	*/
	xxlogline("SP0256: DELTA_9");

	amplitude += nextSL(4) >> 2;
	pitch += nextSL(5) >> 3; // TODO: verify, ob alle 8 bits beeinflusst werden. sollte aber stimmen.

	if (mode & 1) // x1
	{
		B0 += nextSL(4) >> 3;
		F0 += nextSL(4) >> 2;
		B1 += nextSL(4) >> 3;
		F1 += nextSL(4) >> 2;
		B2 += nextSL(4) >> 3;
		F2 += nextSL(4) >> 2;
		B3 += nextSL(4) >> 3;
		F3 += nextSL(5) >> 2;
		B4 += nextSL(5) >> 3;
		F4 += nextSL(5) >> 3;
	}
	else // x0
	{
		B0 += nextSL(3) >> 1;
		F0 += nextSL(3) >> 2;
		B1 += nextSL(3) >> 1;
		F1 += nextSL(3) >> 2;
		B2 += nextSL(3) >> 1;
		F2 += nextSL(3) >> 2;
		B3 += nextSL(3) >> 2;
		F3 += nextSL(4) >> 2;
#if 0
		B4 += nextSL(4)>>2;	F4 += nextSL(4)>>2;	// Zbiciak: 6 bits		i can't hear a difference,
#else
		B4 += nextSL(4) >> 2;
		F4 += nextSL(4) >> 3; // MAME: 7 bits			but i tend to MAME beeing right
#endif
	}

	if (mode & 2)
	{
		B5 += nextSL(5) >> 3;
		F5 += nextSL(5) >> 3;
	}
}


/* 	DELTA_D – Delta update Amplitude, Pitch and 2 or 3 Coefficients
	[data] 1011 RRRR >>>
*/
void SP0256::cmdDeltaD()
{
	/*	Performs a delta update, adding small 2s complement numbers to a series of coefficients.
		The 2s complement updates for the various filter coefficients only update some of the MSBs --
		the LSBs are unaffected.
		The exact bits which are updated are noted below.
		Notes
		• The delta update is applied exactly once, as long as the repeat count is at least 1.
		  If the repeat count is greater than 1, the updated value is held through the repeat period,
		  but the delta update is not reapplied.
		• The delta updates are applied to the 8-bit encoded forms of the coefficients, not the 10-bit decoded forms.
		• Normal 2s complement arithmetic is performed, and no protection is provided against overflow.
		  Adding 1 to the largest value for a bit field wraps around to the smallest value for that bitfield.
		• The update to the amplitude register is a normal 2s complement update to the entire register.
		  This means that any carry/borrow from the mantissa will change the value of the exponent.
		  The update doesn't know anything about the format of that register.

		all modes:
		saaa     spppp      (Amplitude 6 MSBs, Pitch LSBs.)

		mode x0:
		sbb      sfff       (B3 5 MSBs, F3 6 MSBs.)
		sbbb     sfff       (B4 7 MSBs, F4 6 MSBs.)

		mode x1:
		sbbb     sffff      (B3 7 MSBs, F3 7 MSBs.)
		sbbbb    sffff      (B4 8 MSBs, F4 8 MSBs.)

		mode 1x:
		sbbbb    sffff      (B5 8 MSBs, F5 8 MSBs.)
	*/
	xxlogline("SP0256: DELTA_D");

	amplitude += nextSL(4) >> 2;
	pitch += nextSL(5) >> 3; // TODO: verify, ob alle 8 bits beeinflusst werden. sollte aber stimmen.

	if (mode & 1) // x1
	{
		B3 += nextSL(4) >> 3;
		F3 += nextSL(5) >> 2;
		B4 += nextSL(5) >> 3;
		F4 += nextSL(5) >> 3;
	}
	else // x0
	{
		B3 += nextSL(3) >> 2;
		F3 += nextSL(4) >> 2;
		B4 += nextSL(4) >> 3;
		F4 += nextSL(4) >> 2;
	}

	if (mode & 2) // 1x
	{
		B5 += nextSL(5) >> 3;
		F5 += nextSL(5) >> 3;
	}
}


// --------------------------------------------------------------
//					run the co-routine:
// --------------------------------------------------------------

// expand compressed arguments:
inline int ampl(int a) { return (a & 0x1F) << ((a >> 5) & 0x07); }
#if 0
inline int coeff(uint c) { return c&0x80 ? coeff_tab[(~c)&0x7F] : -coeff_tab[c]; } // i can't hear a difference
#else
inline int coeff(uint c) { return c & 0x80 ? coeff_tab[(-c) & 0x7F] : -coeff_tab[c]; } // MAME
#endif


//	Tricky co-routine macros:
#define BEGIN                                                                                                          \
  switch (sm_state)                                                                                                    \
  {                                                                                                                    \
  default: IERR();                                                                                                     \
  case 0:
#define WAIT                                                                                                           \
  do {                                                                                                                 \
	sm_state = __LINE__;                                                                                               \
	return;                                                                                                            \
  case __LINE__:;                                                                                                      \
  }                                                                                                                    \
  while (0)
#define FINISH }


void SP0256::run_statemachine(Time t)
{
	xxlogIn("SP0256::run_statemachine");
	if (t <= time) return;
	assert(t <= seconds_per_dsp_buffer_max());

	BEGIN; // co-routine

	// init micro sequencer:
	mode			= 0;
	page			= 0x1000;
	repeat			= 0;
	stack			= 0;
	stand_by		= yes; // 1 = stand by (utterance completed)
	command_valid	= no;  // 1 = command valid == !LRQ (load request)
	current_command = 0;

	pitch		   = 0;
	amplitude	   = 0;
	pitch_incr	   = 0;
	amplitude_incr = 0;
	memset(c, 0, sizeof(c));
	memset(_c, 0, sizeof(_c));
	memset(_z, 0, sizeof(_z));

	// allophone rom:
	byte = 0; // current/last byte read from rom, remaining valid bits are right-aligned
	bits = 0; // number of remaining valid bits

	// other:
	_shiftreg = 1; // noise


	// run until power off:
	for (;;)
	{
		if (stand_by && command_valid)
		{
			pitch		   = 0;
			amplitude	   = 0;
			pitch_incr	   = 0;
			amplitude_incr = 0;
			memset(c, 0, sizeof(c));

			assert(bits == 0);
			assert(byte == 0);

			xxlogline(
				"SP0256: SBY: next command = %u = %s", command, command < 64 ? al2_allophone_names[command] : "???");
			pc			  = 0x1000 + (command << 1);
			command_valid = no;
			stand_by	  = no;
			if (PRINT_STATISTICS) current_command = command;
		}

		if (stand_by) { repeat = 1; }
		else // load next microcode
		{
			// set/update repeat, pitch, pitch_incr, amplitude, amplitude_incr and
			// filter coefficients c[] & _c[] and clears feed-back values _z[]
			//
			// the sequencer receives a serial bit stream.
			// for the opcode we need 8 bits:
			// the low (1st) nibble contains inline data, the high (2nd) nibble is the instruction
			uint instr	   = next8();
			current_opcode = instr >> 4;

			switch (instr >> 4)
			{
			case SETPAGE: cmdSetPage(instr); continue; // and RTS
			case SETMODE: cmdSetMode(instr); continue;
			case JMP: cmdJmp(instr); continue;
			case JSR: cmdJsr(instr); continue;
			case PAUSE: cmdPause(); break;
			case LOADALL: cmdLoadAll(); break;
			case LOAD_2: cmdLoad2(); break;
			case LOAD_4: cmdLoad4(); break;
			case LOAD_C: cmdLoadC(); break;
			case LOAD_E: cmdLoadE(); break;
			case SETMSB3: cmdSetMsb3(); break;
			case SETMSB5: cmdSetMsb5(); break;
			case SETMSB6: cmdSetMsb6(); break;
			case SETMSBA: cmdSetMsbA(); break;
			case DELTA_9: cmdDelta9(); break;
			case DELTA_D: cmdDeltaD(); break;
			}

			// mockup parameters:
			repeat += (instr & 15);
			assert(repeat < 0x40); // repeat &= 0x3F;		6 bit
			// assert(pitch<0x100);  	// pitch &= 0xff;		8 bit

			// repeat==0 is an ill. condition
			// it does never happen in the AL2 rom
			// it is unclear what the real device did in this case
			if (repeat == 0)
			{
				logline("SP0256: repeat=0");
				continue;
			}

			// convert coefficients: 8 bit -> 10 bit signed
			for (uint i = 0; i < 12; i++)
			{
				_c[i] = coeff(c[i]);
				_z[i] = 0; // clear feed-back values.		((note: verified))
			}
		}

		// for the given number of repetitions:
		for (; repeat; --repeat)
		{
			// for the given number of samples per pulse:
			for (_i = pitch ? pitch : 0x40; _i; _i--)
			{
				// generate next sample:
				// run pulse through filters and update feedback values:
				{
					int z0 = 0; // sample value moving through the filter

					// note: SP0250: pitch.bit6 activates white noise)
					// note: SP0256: pitch==0 activates noise with pitch=64)
					if (pitch == 0) // set z0 from noise:
					{
						// this is the rng from MAME. makes no difference from my version, but anyway:
						_shiftreg = (_shiftreg >> 1) ^ (_shiftreg & 1 ? 0x4001 : 0);
						z0		  = _shiftreg & 1 ? ampl(amplitude) : -ampl(amplitude);
					}
					else // vocal: set z0 for single pulse:
					{
						if (_i == pitch) z0 = ampl(amplitude);
					}

					// apply 6x 2-pole filter:
					for (uint j = 0; j < 12;)
					{
						z0 += _z[j] * _c[j] / 512;
						_z[j] = _z[j + 1];
						j++;
						z0 += _z[j] * _c[j] / 256;
						_z[j] = z0;
						j++;
					}

					if (hifi) // don't limit output to 8 bit
					{
						if (z0 > +0x1FFF) xlogline("filter output > 14 bit");
						if (z0 < -0x2000) xlogline("filter output > 14 bit");
						// limit(-8192,z0,8191);	// 14 bit resolution
					}
					else // original 8 bit resolution
					{
						limit(-2048, z0, 2047);
						z0 &= ~0xF;
					}

					sample = z0 * amplification;
				}

				// output one sample:
				// es scheint so, als ob ein Filterdurchlauf 312 Takte dauert
				// Die Ausgabefrequenz ist dann genau 10 kHz
				// Note: SP0256 S.13: Speech Code Generation:
				//	 "The analog speech signal from a tape recording is … sampled at a 10 kHz rate."

				// bis this.time haben wir schon sound ausgegeben
				// jetzt ein sample anhängen, max. aber bis t

				time_sample_end = time + time_per_sample;

				while (time_sample_end > t) // dieses Sample bringt den Puffer zum Überlauf
				{
					output_filtered(sample, t);
					WAIT; // co-routine
				}

				// whole|remaining sample fits in remaining time or buffer[]
				{
					output_filtered(sample, time_sample_end);
				}
			}

			// apply increments:
			pitch += pitch_incr;
			amplitude += amplitude_incr;
		}
	}

	FINISH; // co-routine
}


/*	output samples to DSP output buffer with double RC filter
	• Filtert und speichert den übergebenen Sample-Wert ab dem letzten Zeitpunkt in this.time
	  bis zum angegebenen neuen Zeitpunkt ee.
	• ee darf nicht über den dsp output buffer (plus stitching) hinausgehen
	• Aktualisiert time aus ee.
	• Das Filter hat bei 1 kHz eine Dämpfung von ca. 24 dB, also etwas etwa: Sample(out) = Sample(in) / 12.
	  Das sollte vorkompensiert werden.
	  Die untere Übergangsfrequenz (-3 dB) ist 139 Hz.

	filter with RC = 33kΩ*22nF which allegedly is 5kHz cut-off frequency
	RC = 726e-6
	1/RC = 1377.41

	dU/dt = U * 1/RC
		  = U * 1377

	normiert auf U = 1V:
	dU/dt = 1/RC = 1377

	y = e^(-x) hat bei x=0 den Wert y=1
	U = e^(-t) hat bei t=0 den Wert U=1
	U = e^(-t) hat bei t=1 den Wert U=e^(-1) = 1/e
	U = e^(-t) hat bei t=a den Wert U=e^(-a) = a/e
	Die Steigung in t=0 ist -1

	Wir haben aber eine Steigung von
		dU/dt = -1/RC										dU/dt = -1377
	Das entspricht einem hor. Skalierungsfaktor von
		1/(1/RC) = RC										RC = 1/1377
	U = e^(-t) hat bei t=RC den Wert U=e^(-1) = 1/e			t = 1/1377
	U = e^(-t) hat bei t=aRC den Wert U=e^(-a) = 1/e^a		t = a/1377

		t = aRC <=> a = t/RC

	für t = seconds_per_sample = sps						sps = 1/44.1e3
		a = 1/RC											a = 1/(44.1e+3*726e-6) = 3.123e-2
	=>	U = e^(-t) hat bei t=sps = t=aRC					t = 3.123e-2/1377 = 2.268e-5 = 1/44100
		den Wert U(sps) = e^(-a)							U(sps) = e^(-3.123e-2) = 0.96925261933488

	für t = cutoff = 1 / 5kHz
		a = t/RC
		a = cutoff/RC = 1/5000/RC							a = 0.27548209366391
	=>	U = e^(-t) hat bei t=cutoff = t=aRC					t = 1/5000
		den Wert U(cutoff) = e^(-a)							U(cutoff) = e^(-0.27548209366391) = 0.75920602657061

		d.h. nach 1/5000 sec hat sich der Wert erst um 24% dem Zielwert angenähert.
		Das scheint etwas wenig.
		Vor allem, wo in der Applikation dieser Filter 2x hintereinandergeschaltet ist.
		hmm...
			Übergangsfrequenz fc = 1 / (2πRC) = 219.22 Hz
			Bei der Übergangsfrequenz fc (Grenzfrequenz) einer Absenkung ist die Spannung U
			immer auf 1/√2 = 0,7071 ≡ 70.71% abgefallen und der Spannungspegel ist dort um
			20·log10(1/√2) = (−)3.0103 dB gedämpft.

			Die Zeitkonstante τ eines RC-Glieds ist so definiert:
			Zeitkonstante = Kapazität * Zeit
			τ = R * C
			1 s = 1 Ohm * 1 F
			In der Zeit τ hat sich der Kondensator auf 1 - 1/e = 63,2% der Ladespannung aufgeladen
			bzw. bis auf 1/e = 36,8% entladen.
			τ = RC = 726µs

		http://www.electronicdeveloper.de/FilterPassivTiefpassRCPWM1.aspx
		2-stufiger RC-Filter:					1-stufiger RC-Filter:
		33kΩ * 22nF + 33kΩ * 22nF				33kΩ * 22nF			66kΩ * 22nF
		->  Bandbreite (-3dB) = 138.8 Hz		219.222 Hz			109.6 Hz
			Dämpfung 12 dB / Oktave				6 dB/Oktave			6 dB/Oktave
					 40 dB / Dekade				20 dB/Dekade		20 dB/Dekade
			Dämpfung bei 10kHz: -64.27 dB		-29.26 dB
					 bei 5 kHz: -52.24 dB		-23.25 dB
					 bei 2 kHz:	-36.41 dB		-15.37 dB
					 bei 1 kHz: -24.67 dB		-9.59 dB
					 bei 500Hz: -13.75 dB		-4.48 dB
					 bei 250Hz: -5.14  dB		-1.11 dB

		Bei der SP0250-Applikation gibt es noch zu bedenken, dass der Ausgang des Filters in einen
		vergleichsweise niederohmigen Eingang (10kΩ) läuft.
		hmm...
*/
void SP0256::output_filtered(Sample sample, Time ee)
{
	Time aa = time; // aa = start time
	time	= ee;	// ee = end time

	aa *= samples_per_second; // index in audio_out_buffer[] (fractional)
	ee *= samples_per_second; // index in audio_out_buffer[] (fractional)

	int32 a = int32(aa); // index in audio_out_buffer[] (integer)
	int32 e = int32(ee); // index in audio_out_buffer[] (integer)

	assert(a >= 0);
	assert(e >= a);
	assert(e < DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING);

	//	Time sps = 1.0 / samples_per_second;	// Zeit für ein Sample (seconds per sample)
	//	float64 RC = 33e3 * 22e-9;				// Konstante aus R/Ω und C/F
	//	float64 a = sps/RC;						// "horizontaler Skalierungsfaktor"
	//	float64 U = exp(-a);					// Wert der skalierten e-Funktion nach t = sps
	//	float64 f = 1.0 - U;					// Das Gegenstück, die Änderung:
	//											// Um diesen Bruchteil ändert sich im Verlauf eines Samples
	//											// der Ausgangslevel in Richtung des Zielwertes
	//
	//	float64 ff = 1.0 - exp( -(1.0/samples_per_second) / (33e3 * 22e-9) );


	float  ff2; // scratch: partial ff for partial samples
	Sample d;

	if (a == e)
	{
		ff2 = ff * float(ee - aa); // note: im sub-Sample-Bereich wird ff einfach interpoliert
		sample_at_c1 += (sample - sample_at_c1) * ff2;
		sample_at_c2 += d = (sample_at_c1 - sample_at_c2) * ff2;
		sample_at_c1 -= d;
		if (fabsf(sample_at_c2) > 2.0f) xxlog("sample>2");
		Dsp::audio_out_buffer[a] += sample_at_c2 * float(ee - aa);
	}
	else
	{
		ff2 = ff * float(1.0 - (aa - floor(aa)));
		sample_at_c1 += (sample - sample_at_c1) * ff2;
		sample_at_c2 += d = (sample_at_c1 - sample_at_c2) * ff2;
		sample_at_c1 -= d;
		if (fabsf(sample_at_c2) > 2.0f) xxlog("sample>2");
		Dsp::audio_out_buffer[a++] += sample_at_c2 * (1.0f - float(aa - floor(aa)));

		while (a < e)
		{
			sample_at_c1 += (sample - sample_at_c1) * ff;
			sample_at_c2 += d = (sample_at_c1 - sample_at_c2) * ff;
			sample_at_c1 -= d;
			if (fabsf(sample_at_c2) > 2.0f) xxlog("sample>2");
			Dsp::audio_out_buffer[a++] += sample_at_c2;
		}

		ff2 = ff * float(ee - floor(ee));
		sample_at_c1 += (sample - sample_at_c1) * ff2;
		sample_at_c2 += d = (sample_at_c1 - sample_at_c2) * ff2;
		sample_at_c1 -= d;
		if (fabsf(sample_at_c2) > 2.0f) xxlog("sample>2");
		Dsp::audio_out_buffer[a] += sample_at_c2 * float(ee - floor(ee));
	}

	if (PRINT_STATISTICS && current_opcode < 16)
	{
		Stats& s = opcode_stats[current_opcode];
		if (fabs(sample) > s.max_sample) s.max_sample = fabs(sample);
		if (fabs(sample_at_c1) > s.max_sample_at_c1) s.max_sample_at_c1 = fabs(sample_at_c1);
		if (fabs(sample_at_c2) > s.max_sample_at_c2) s.max_sample_at_c2 = fabs(sample_at_c2);
	}
	if (PRINT_STATISTICS && current_command < 64)
	{
		Stats& s = allophone_stats[current_command];
		if (fabs(sample) > s.max_sample) s.max_sample = fabs(sample);
		if (fabs(sample_at_c1) > s.max_sample_at_c1) s.max_sample_at_c1 = fabs(sample_at_c1);
		if (fabs(sample_at_c2) > s.max_sample_at_c2) s.max_sample_at_c2 = fabs(sample_at_c2);
	}
}


void SP0256::disassAllophones()
{
	logIn("SP0256: allophone rom disassembly");

	for (uint i = 0; i < 64; i++)
	{
		logIn("allophone %2u: %s", i, al2_allophone_names[i]);

		pc			  = i << 1;
		byte		  = 0;
		bits		  = 0;
		repeat		  = 0;
		stack		  = 0;
		mode		  = 0;
		page		  = 0;
		command_valid = no;
		stand_by	  = no;

		while (!stand_by)
		{
			uint instr	   = next8();
			current_opcode = instr >> 4;

			switch (current_opcode)
			{
			case SETPAGE: cmdSetPage(instr); continue; // and RTS
			case SETMODE: cmdSetMode(instr); continue;
			case JMP: cmdJmp(instr); continue;
			case JSR: cmdJsr(instr); continue;
			}

			repeat += (instr & 15);
			log("%2u: %s: m=%u, r=%u%s ",
				current_opcode,
				opcode_names[current_opcode],
				mode,
				repeat,
				repeat > 9 ? "" : " ");
			repeat = 0;

			switch (current_opcode)
			{
			case PAUSE: logPause(); break;
			case LOADALL: IERR(); break;
			case LOAD_2: logLoad2(); break;
			case LOAD_4: logLoad4(); break;
			case LOAD_C: logLoadC(); break;
			case LOAD_E: logLoadE(); break;
			case SETMSB3: logSetMsb3(); break;
			case SETMSB5: logSetMsb5(); break;
			case SETMSB6: IERR(); break;
			case SETMSBA: logSetMsbA(); break;
			case DELTA_9: logDelta9(); break;
			case DELTA_D: logDeltaD(); break;
			}

			assert(pitch_incr == 0);
			assert(amplitude_incr == 0);
			logNl();
		}
	}
}


#define AMP(A) (((A)&0x1F) << (((A) >> 5) & 0x07))
#define COF(N) ((N)&0x80 ? coeff_tab[(~(N)) & 0x7F] : -coeff_tab[N])


/*	PAUSE – Silent pause
	1111 RRRR >>>
*/
void SP0256::logPause()
{
	/*	Provides a silent pause of varying length.
		The length of the pause is given by the 4-bit immediate constant RRRR.
		The pause duration can be extended with the prefix opcode SETMODE.
		Notes: The pause behaves identially to a pitch with Amplitude=0 and Period=64.
		All coefficients are cleared.
	*/
	amplitude	   = 0;
	pitch		   = 64;
	amplitude_incr = 0; // not expressively stated, but otherwise pause with R>1 would not be silent
	pitch_incr	   = 0; // not expressively stated, but otherwise pause with R>1 would not be silent
	memset(c, 0, sizeof(c));

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
}


/*	LOAD_E – Load Pitch and Amplitude
	PPPPPPPP AAAAAA 0111 RRRR >>>
*/
void SP0256::logLoadE()
{
	/*	Load new amplitude and pitch.
		There seem to be no data format variants for the different modes,
		although the repeat count may be extended using opcode SETMODE.

		probably: all other registers preserved.
	*/
	amplitude = nextL(6);
	pitch	  = next8();

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
}


/*	LOAD_4 – Load Pitch, Amplitude and Coefficients (2 or 3 stages)
	[data] 0010 RRRR >>>
*/
void SP0256::logLoad4()
{
	/*	Load new amplitude and pitch parameters.
		Load new filter coefficients, setting the unspecified coefficients to 0.
		The exact combination and precision of data is determined by the mode bits as set by opcode SETMODE.
		For all modes, the sign bit for B0 has an implied value of 0.

		all modes:
		AAAAAA   PPPPPPPP

		mode x0:
		BBBB     SFFFFF     (B3,F3)
		SBBBBBB  SFFFFF     (B4,F4)

		mode x1:
		BBBBBB   SFFFFFF    (B3,F3)
		SBBBBBBB SFFFFFFF   (B4,F4)

		mode 1x:
		SBBBBBBB SFFFFFFF   (B5,F5)
	*/
	amplitude	   = nextL(6);
	pitch		   = next8();
	amplitude_incr = 0;
	pitch_incr	   = 0;

	memset(c, 0, sizeof(c)); // set the unspecified coefficients to 0

	if (mode & 1) // x1
	{
		B3 = nextL(6) >> 1;
		F3 = nextL(7);
		B4 = next8();
		F4 = next8();
	}
	else // x0
	{
		B3 = nextL(4) >> 1;
		F3 = nextL(6);
		B4 = nextL(7);
		F4 = nextL(6);
	}

	if (mode & 2) // 1x
	{
		B5 = next8();
		F5 = next8();
	}

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
	log("F0=%+4i,%+4i ", COF(B0), COF(F0));
	log("F1=%+4i,%+4i ", COF(B1), COF(F1));
	log("F2=%+4i,%+4i ", COF(B2), COF(F2));
	log("F3=%+4i,%+4i ", COF(B3), COF(F3));
	log("F4=%+4i,%+4i ", COF(B4), COF(F4));
	log("F5=%+4i,%+4i ", COF(B5), COF(F5));
}


/*	LOAD_C – Load Pitch, Amplitude, Coefficients (5 or 6 stages)
	LOAD_2 – Load Pitch, Amplitude, Coefficients (5 or 6 stages), and Interpolation Registers
	[data] 0011 RRRR >>>
	[data] 0100 RRRR >>>
*/
void SP0256::logLoad2C(uint instr)
{
	/*	Load new amplitude and pitch parameters.
		Load new filter coefficients, setting the unspecified coefficients to zero.
		The exact combination and precision of data is determined by the mode bits as set by opcode SETMODE.
		• For all Modes, the Sign bit for B0, B1, B2 and B3 has an implied value of 0.
		• LOAD_2 and LOAD_C only differ in that LOAD_2 also loads new values into the
		  Amplitude and Pitch Interpolation Registers while LOAD_C doesn't.

		all modes:
		AAAAAA   PPPPPPPP

		mode x0:
		BBB      SFFFF      (coeff pair 0)
		BBB      SFFFF      (coeff pair 1)
		BBB      SFFFF      (coeff pair 2)
		BBBB     SFFFFF     (coeff pair 3)
		SBBBBBB  SFFFFF     (coeff pair 4)

		mode x1:
		BBBBBB   SFFFFF     (coeff pair 0)
		BBBBBB   SFFFFF     (coeff pair 1)
		BBBBBB   SFFFFF     (coeff pair 2)
		BBBBBB   SFFFFFF    (coeff pair 3)
		SBBBBBBB SFFFFFFF   (coeff pair 4)

		mode 1x:
		SBBBBBBB SFFFFFFF   (coeff pair 5)

		LOAD_2 only, all modes:
		aaaaa    ppppp      (Interpolation register LSBs)
	*/
	amplitude = nextL(6);
	pitch	  = next8();

	if (mode & 1) // x1
	{
		B0 = nextL(6) >> 1;
		F0 = nextL(6);
		B1 = nextL(6) >> 1;
		F1 = nextL(6);
		B2 = nextL(6) >> 1;
		F2 = nextL(6);
		B3 = nextL(6) >> 1;
		F3 = nextL(7);
		B4 = next8();
		F4 = next8();
	}
	else // x0
	{
		B0 = nextL(3) >> 1;
		F0 = nextL(5);
		B1 = nextL(3) >> 1;
		F1 = nextL(5);
		B2 = nextL(3) >> 1;
		F2 = nextL(5);
		B3 = nextL(4) >> 1;
		F3 = nextL(6);
		B4 = nextL(7);
		F4 = nextL(6);
	}
	if (mode & 2) // 1x
	{
		B5 = next8();
		F5 = next8();
	}
	else // 0x
	{
		B5 = 0;
		F5 = 0;
	}

	if (instr == LOAD_2) // LOAD_2
	{
		amplitude_incr = nextR(5);
		pitch_incr	   = nextR(5);
	}
	else // LOAD_C
	{
		amplitude_incr = 0;
		pitch_incr	   = 0;
	}

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
	log("F0=%+4i,%+4i ", COF(B0), COF(F0));
	log("F1=%+4i,%+4i ", COF(B1), COF(F1));
	log("F2=%+4i,%+4i ", COF(B2), COF(F2));
	log("F3=%+4i,%+4i ", COF(B3), COF(F3));
	log("F4=%+4i,%+4i ", COF(B4), COF(F4));
	log("F5=%+4i,%+4i ", COF(B5), COF(F5));
}


/*	LOAD_2 – Load Pitch, Amplitude, Coefficients, and Interpolation Registers
	[data] 0100 RRRR >>>
*/
inline void SP0256::logLoad2() { logLoad2C(LOAD_2); }

/*	LOAD_C – Load Pitch, Amplitude, Coefficients (5 or 6 stages)
	[data] 0011 RRRR >>>
*/
inline void SP0256::logLoadC() { logLoad2C(LOAD_C); }


/*	SETMSB_5 – Load Amplitude, Pitch, and MSBs of 3 Coefficients
	SETMSB_A – Load Amplitude,        and MSBs of 3 Coefficients
	SETMSB_3 – Load Amplitude,        and MSBs of 3 Coefficients, and Interpolation Registers
	[data] 1010 RRRR >>>
	[data] 0101 RRRR >>>
	[data] 1100 RRRR >>>
*/
void SP0256::logSetMsb35A(uint instr)
{
	/*	Load new amplitude and pitch parameters.
		Update the MSBs of a set of filter coefficients.
		The Mode bits control the update process as noted below. Opcode SETMODE provides the mode bits.
		Notes:
		MODE x0: Set the 5 or 6 msbits of F0, F1, and F2, the lsbits are preserved.
		MODE x1: Set the 5 or 6 msbits of F0, F1, and F2, the lsbits are preserved.
		MODE 0x: F5 and B5 are set to 0. All other coefficients are preserved.
		MODE 1x: F5 and B5 are preserved. All other coefficients are preserved.
		Opcode SETMSB_5 also sets the Pitch.
		Opcode SETMSB_3 also sets the interpolation registers.

		all opcodes, all modes:
		AAAAAA

		SETMSB_5, all modes:
		PPPPPPPP

		all opcodes, mode x0:
		SFFFF			(F0 5 MSBs)
		SFFFF			(F1 5 MSBs)
		SFFFF			(F2 5 MSBs)

		all opcodes, mode x1:
		SFFFFF			(F0 6 MSBs)
		SFFFFF			(F1 6 MSBs)
		SFFFFF			(F2 6 MSBs)

		SETMSB_3, all modes:
		aaaaa    ppppp	(incr. LSBs)
	*/
	amplitude = nextL(6);
	if (instr == SETMSB5) { pitch = next8(); }

	if (mode & 1) // x1
	{
		F0 = nextL(6) + (F0 & 3);
		F1 = nextL(6) + (F1 & 3);
		F2 = nextL(6) + (F2 & 3);
	}
	else // x0
	{
		F0 = nextL(5) + (F0 & 7);
		F1 = nextL(5) + (F1 & 7);
		F2 = nextL(5) + (F2 & 7);
	}

	if (mode & 2) // 1x
	{}
	else // 0x
	{
		F5 = B5 = 0;
	}

	if (instr == SETMSB3)
	{
		amplitude_incr = nextR(5);
		pitch_incr	   = nextR(5);
	}

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
	log("F0=%+4i,%+4i ", COF(B0), COF(F0));
	log("F1=%+4i,%+4i ", COF(B1), COF(F1));
	log("F2=%+4i,%+4i ", COF(B2), COF(F2));
	log("F5=%+4i,%+4i ", COF(B5), COF(F5));
}


/*	SETMSB_3 – Load Amplitude, MSBs of 3 Coefficients, and Interpolation Registers
	[data] 1100 RRRR >>>
*/
inline void SP0256::logSetMsb3() { logSetMsb35A(SETMSB3); }

/*	SETMSB_5 – Load Amplitude, Pitch, and MSBs of 3 Coefficients
	[data] 1010 RRRR >>>
*/
inline void SP0256::logSetMsb5() { logSetMsb35A(SETMSB5); }

/*	SETMSB_A – Load Amplitude and MSBs of 3 Coefficients
	[data] 0101 RRRR >>>
*/
inline void SP0256::logSetMsbA() { logSetMsb35A(SETMSBA); }


/*	DELTA_9 – Delta update Amplitude, Pitch and 5 or 6 Coefficients
	[data] 1001 RRRR
*/
void SP0256::logDelta9()
{
	/*	Performs a delta update, adding small 2s complement numbers to a series of coefficients.
		The 2s complement updates for the various filter coefficients only update some of the MSBs,
		the LSBs are unaffected.
		The exact bits which are updated are noted below.
		Notes
		• The delta update is applied exactly once, as long as the repeat count is at least 1.
		• If the repeat count is greater than 1, the updated value is held through the repeat period,
		  but the delta update is not reapplied.
		• The delta updates are applied to the 8-bit encoded forms of the coefficients, not the 10-bit decoded forms.
		  Normal 2s complement arithmetic is performed, and no protection is provided against overflow.
		  Adding 1 to the largest value for a bit field wraps around to the smallest value for that bitfield.
		• The update to the amplitude register is a normal 2s complement update to the 8-bit register.
		  This means that any carry/borrow from the mantissa will change the value of the exponent.
		  The update doesn't know anything about the format of that register.

		all modes:
		saaa     spppp      (Amplitude 6 MSBs, Pitch LSBs.)

		mode x0:
		sbb      sff        (B0 4 MSBs, F0 5 MSBs.)
		sbb      sff        (B1 4 MSBs, F1 5 MSBs.)
		sbb      sff        (B2 4 MSBs, F2 5 MSBs.)
		sbb      sfff       (B3 5 MSBs, F3 6 MSBs.)
		sbbb     sfff       (B4 6 MSBs, F4 6 MSBs.)	// TODO: MAME: B4 7 Bits!

		mode x1:
		sbbb     sfff       (B0 7 MSBs, F0 6 MSBs.)
		sbbb     sfff       (B1 7 MSBs, F1 6 MSBs.)
		sbbb     sfff       (B2 7 MSBs, F2 6 MSBs.)
		sbbb     sffff      (B3 7 MSBs, F3 7 MSBs.)
		sbbbb    sffff      (B4 8 MSBs, F4 8 MSBs.)

		mode 1x:
		sbbbb    sffff      (B5 8 MSBs, F5 8 MSBs.)
	*/
	amplitude += nextSL(4) >> 2;
	pitch += nextSL(5) >> 3; // TODO: verify, ob alle 8 bits beeinflusst werden. sollte aber stimmen.

	if (mode & 1) // x1
	{
		B0 += nextSL(4) >> 3;
		F0 += nextSL(4) >> 2;
		B1 += nextSL(4) >> 3;
		F1 += nextSL(4) >> 2;
		B2 += nextSL(4) >> 3;
		F2 += nextSL(4) >> 2;
		B3 += nextSL(4) >> 3;
		F3 += nextSL(5) >> 2;
		B4 += nextSL(5) >> 3;
		F4 += nextSL(5) >> 3;
	}
	else // x0
	{
		B0 += nextSL(3) >> 1;
		F0 += nextSL(3) >> 2;
		B1 += nextSL(3) >> 1;
		F1 += nextSL(3) >> 2;
		B2 += nextSL(3) >> 1;
		F2 += nextSL(3) >> 2;
		B3 += nextSL(3) >> 2;
		F3 += nextSL(4) >> 2;
		B4 += nextSL(4) >> 2;
		F4 += nextSL(4) >> 3; // Zbiciak: 6 bits, MAME: 7 bits
	}

	if (mode & 2)
	{
		B5 += nextSL(5) >> 3;
		F5 += nextSL(5) >> 3;
	}

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
	log("F0=%+4i,%+4i ", COF(B0), COF(F0));
	log("F1=%+4i,%+4i ", COF(B1), COF(F1));
	log("F2=%+4i,%+4i ", COF(B2), COF(F2));
	log("F3=%+4i,%+4i ", COF(B3), COF(F3));
	log("F4=%+4i,%+4i ", COF(B4), COF(F4));
	log("F5=%+4i,%+4i ", COF(B5), COF(F5));
}


/* 	DELTA_D – Delta update Amplitude, Pitch and 2 or 3 Coefficients
	[data] 1011 RRRR >>>
*/
void SP0256::logDeltaD()
{
	/*	Performs a delta update, adding small 2s complement numbers to a series of coefficients.
		The 2s complement updates for the various filter coefficients only update some of the MSBs --
		the LSBs are unaffected.
		The exact bits which are updated are noted below.
		Notes
		• The delta update is applied exactly once, as long as the repeat count is at least 1.
		  If the repeat count is greater than 1, the updated value is held through the repeat period,
		  but the delta update is not reapplied.
		• The delta updates are applied to the 8-bit encoded forms of the coefficients, not the 10-bit decoded forms.
		• Normal 2s complement arithmetic is performed, and no protection is provided against overflow.
		  Adding 1 to the largest value for a bit field wraps around to the smallest value for that bitfield.
		• The update to the amplitude register is a normal 2s complement update to the entire register.
		  This means that any carry/borrow from the mantissa will change the value of the exponent.
		  The update doesn't know anything about the format of that register.

		all modes:
		saaa     spppp      (Amplitude 6 MSBs, Pitch LSBs.)

		mode x0:
		sbb      sfff       (B3 5 MSBs, F3 6 MSBs.)
		sbbb     sfff       (B4 7 MSBs, F4 6 MSBs.)

		mode x1:
		sbbb     sffff      (B3 7 MSBs, F3 7 MSBs.)
		sbbbb    sffff      (B4 8 MSBs, F4 8 MSBs.)

		mode 1x:
		sbbbb    sffff      (B5 8 MSBs, F5 8 MSBs.)
	*/
	amplitude += nextSL(4) >> 2;
	pitch += nextSL(5) >> 3; // TODO: verify, ob alle 8 bits beeinflusst werden. sollte aber stimmen.

	if (mode & 1) // x1
	{
		B3 += nextSL(4) >> 3;
		F3 += nextSL(5) >> 2;
		B4 += nextSL(5) >> 3;
		F4 += nextSL(5) >> 3;
	}
	else // x0
	{
		B3 += nextSL(3) >> 2;
		F3 += nextSL(4) >> 2;
		B4 += nextSL(4) >> 3;
		F4 += nextSL(4) >> 2;
	}

	if (mode & 2) // 1x
	{
		B5 += nextSL(5) >> 3;
		F5 += nextSL(5) >> 3;
	}

	log("p=%2u a=%4u  ", pitch, AMP(amplitude));
	log("F3=%+4i,%+4i ", COF(B3), COF(F3));
	log("F4=%+4i,%+4i ", COF(B4), COF(F4));
	log("F5=%+4i,%+4i ", COF(B5), COF(F5));
}
