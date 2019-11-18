/*	Copyright  (c)	Günter Woigk 2000 - 2018
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

#define LOGLEVEL 2
#define SAFETY 1
#include <zlib.h>
#include <math.h>
#include "TapeFile.h"
#include "TzxData.h"
#include "RlesData.h"
#include "TapData.h"
#include "O80Data.h"
#include "TapeFileDataBlock.h"
#include "kio/standard_types.h"
#include "kio/util/count1bits.h"
#include "globals.h"
#include "kio/peekpoke.h"
class FD;



// timing constants:
//
static Time	  const SPCC			= 1.0/3500000;	// seconds per cpu cycle @ 3.5MHz

// timing constants for ZX Spectrum:
//
static uint32 const	cc_zxsp_pilot	= 2168;			// cpu cycles per pilot half phase
static uint32 const	cc_zxsp_bit0	=  855;			// cpu cycles per 0-bit half phase
static uint32 const	cc_zxsp_bit1	= 1710;			// cpu cycles per 1-bit half phase
static uint32 const	cc_zxsp_sync1	=  667;			// cpu cycles for 1st sync phase
static uint32 const	cc_zxsp_sync2	=  735;			// cpu cycles for 2nd sync phase

//static uint const	ppulses_zx_0x00 = 8063;         // num pilot pulses: header; odd due to invisible 1st pulse
//static uint const	ppulses_zx_0x80 = 3223;         // num pilot pulses: data;	 odd due to invisible 1st pulse


// timing constants for Jupiter Ace:
//
static uint32 const	cc_jup_pilot = 2011 *350/325;	// 2011 @ 3.25 MHz
static uint32 const	cc_jup_sync1 = 601  *350/325;	// 601
static uint32 const	cc_jup_sync2 = 791  *350/325;	// 791
static uint32 const	cc_jup_bit0  = 798  *350/325;	// 1:795,  0:801
static uint32 const	cc_jup_bit1  = 1588 *350/325;	// 1:1585, 0:1591

//static uint const	ppulses_jup_0x00 = 8192;		// num pilot pulses: header; even
//static uint const	ppulses_jup_0x80 = 1024;		// num pilot pulses: data;	 even



// -------------------------------------------------------------
//      Helper
// -------------------------------------------------------------

//	switch(id)		// Supported blocks:
//	{
//		case 0x10:	// Standard speed data block
//		case 0x11:	// Turbo speed data block
//		case 0x12:	// Pure tone
//		case 0x13:	// Sequence of pulses of various lengths
//		case 0x14:	// Pure data block
//		case 0x15:	// Direct recording block
//		case 0x18:	// CSW recording block
//		case 0x19:	// Generalized data block
//		case 0x20:	// Pause (silence) or 'Stop the tape' command
//		case 0x21:	// Group start
//		case 0x22:	// Group end
//		case 0x24:	// Loop start
//		case 0x25:	// Loop end
//		case 0x2A:	// Stop the tape if in 48K mode
//		case 0x2B:	// Set signal level
//		case 0x30:	// Text description
//		case 0x31:	// Message block
//		case 0x32:	// Archive info
//		case 0x33:	// Hardware type
//	}



// -------------------------------------------------------------
//          Base class for the various TZX blocks:
//
//	Ein TapeData-Block ist eine Daten-Einheit, die durch eine Pause von anderen Einheiten abgegrenzt ist.
//	Ein TzxData-Block kann aus mehreren TzxBlocks bestehen. TzxBlocks sind die Building Blocks des Tzx-Files.
//	Lt. TZX-Spec ist die Initial Phase des ersten Blocks auf einem Band "low".
//	Lt. Tzx-Specs ist die Initial Phase des nächsten Blocks nach einer Pause "low".
//	Ein TapeData-Block startet also immer mit einer Initial Phase von "low" == 0.
//
// -------------------------------------------------------------

class TzxBlock
{
public:
	uint		id;			// Tzx Block ID
	TzxBlock*	next;		// linked list if a TapeData block consists of more than 1 TzxBlock
	bool		has_data;	// will write(CswBuffer) actually write pulses?

// *** interface ***
private:
virtual	void    read        (FD&)				  =0;	// read this block from file, except id-byte
virtual void    write       (FD&)			const =0;	// write this block to file, except id-byte
virtual void    write		(CswBuffer&)	const {}	// store this block to CSW buffer
public:
virtual cstr    get_info	()				const { return NULL; }	// get major block info of this block, if any
virtual cstr    get_info2	()				const { return NULL; }	// get minor block info of this block, if any
virtual bool    is_end_block()				const { return no; }	// does it end with a pause?

// helper:
static	TzxBlock* read_next	(FD& fd)		noexcept(false); // data_error, file_error
		TzxBlock* last		()				{ TzxBlock* p = this; while(p->next) p = p->next; return p; }

public:
				TzxBlock    (uint id, bool has_data)	:id(id),next(NULL),has_data(has_data){}
virtual			~TzxBlock   ()							{ delete next; }

		// read a (linked list of) TzxBlocks from file
static	TzxBlock* readFromFile(FD&, uint stopper=0)	noexcept(false); // data_error, file_error

		// write a (liked list of) TzxBlocks to file
		void writeToFile (FD&)				const noexcept(false); // data_error

		// render a (linked list of) TzxBlocks into a CswBuffer
		void storeCsw(CswBuffer&)			const;

		// append block at end of (list of) this block
		TzxBlock* append(TzxBlock* b)		{ TzxBlock* p=this; while(p->next) p=p->next; p->next=b; return this; }
		TzxBlock& operator+	(TzxBlock* b)	{ return *append(b); }
};


/*  read block from tzx file:
	read next TzxBlock and all following blocks until EOF or a stopper is found
		stopper=0: block.is_end_block()
		stopper≠0: block.id==stopper
	all blocks are linked in a list via 'next'.
	returns NULL at eof
*/
TzxBlock* TzxBlock::readFromFile( FD& fd, uint stopper ) noexcept(false) // data_error, file_error
{
	TzxBlock* result = read_next(fd);
	TzxBlock* p      = result;

	while(p)	// p==NULL -> eof
	{
		if(p->id==0x22 && stopper!=0x22) throw data_error("tzx file: unexpected block 0x22 GROUP_END");
		if(p->id==0x25 && stopper!=0x25) throw data_error("tzx file: unexpected block 0x25 LOOP_END");

		if(stopper) { if(p->id==stopper) break; }
		else		{ if(p->is_end_block()) break; }

		p = p->next = read_next(fd);
	}

	return result;
}

void TzxBlock::writeToFile( FD& fd ) const noexcept(false) // data_error
{
	for(TzxBlock const* p = this; p; p=p->next)
	{
		fd.write_uint8(p->id);
		p->write(fd);
	}
}

void TzxBlock::storeCsw( CswBuffer& bu ) const
{
	for(TzxBlock const* p = this; p; p=p->next)
	{
		p->write(bu);
	}
}



// -------------------------------------------------------------
//          Implementations for the various TZX blocks:
// -------------------------------------------------------------



// -------------------------------------------------------------
// Block 0x10: Standard speed data block

/*  Offset  Value 	Type 	Description
	0x00 	-       WORD 	Pause after this block (ms.) {1000}
	0x02 	N       WORD    Length of data that follow
	0x04 	-       BYTE[N] Data as in .TAP files

	This block must be replayed with the standard Spectrum ROM timing values.
	The pilot tone consists in 8063 pulses if the first data byte (flag byte) is < 128, 3223 otherwise.
	This block can be used for the ROM loading routines AND for custom loading routines
	that use the same timings as ROM ones do.

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

struct TzxBlock10 : public TzxBlock
{
	uint16  pause_ms;   // pause after this block [ms] {1000}
	uint16  cnt;        // num. bytes that follow
	uint8*	data;

	TzxBlock10()				:TzxBlock(0x10,yes),data(NULL){}
	TzxBlock10(TapData const&);
	~TzxBlock10()				{ delete[] data; }
	void read	(FD& fd);
	void write	(FD& fd)		const;
	void write	(CswBuffer&)	const;
	bool is_end_block()			const { return pause_ms!=0; }
	// get_info() and get_info2() need not be implemented,
	// because TapeFileDataBlock will convert TzxData to TapData and get it there
};


void TzxBlock10::read(FD &fd)
{
	xlogIn("Block 0x10: standard tape block");
	assert(data==NULL);

	pause_ms = fd.read_uint16_z();
	cnt      = fd.read_uint16_z();	if(cnt==0) throw data_error("tzx block 0x10: count==0");
	data     = new uint8[cnt]; fd.read_bytes(data,cnt);
}

void TzxBlock10::write(FD &fd) const
{
	fd.write_uint16_z(pause_ms);
	fd.write_uint16_z(cnt);
	fd.write_bytes(data,cnt);
}

void TzxBlock10::write(CswBuffer& bu) const
{
	// we expect phase 0 (as after TzxPause)
	// phase 1 might be an indication for a Jupiter Ace block,
	// but the tzx format specifies the number of pilot pulses for this block

	if(bu.getCurrentPhase()) xlogline("tzx block 0x10: inverted signal");

	bu.writePureTone( data[0]&0x80 ? 3223 : 8063, cc_zxsp_pilot*SPCC );		// as specified
	bu.writePulse   ( cc_zxsp_sync1 * SPCC );
	bu.writePulse   ( cc_zxsp_sync2 * SPCC );
	bu.writePureData( data, cnt*8, cc_zxsp_bit0*SPCC, cc_zxsp_bit1*SPCC );
	bu.writeTzxPause( pause_ms*0.001 );
}


// -------------------------------------------------------------
// Block 0x11: Turbo speed data block

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Length of PILOT pulse {2168}
	0x02 	-       WORD 	Length of SYNC first pulse {667}
	0x04 	-       WORD 	Length of SYNC second pulse {735}
	0x06 	-       WORD 	Length of ZERO bit pulse {855}
	0x08 	-       WORD 	Length of ONE bit pulse {1710}
	0x0A 	-       WORD 	Length of PILOT tone (number of pulses) {3223} or {8063}
	0x0C 	-       BYTE 	Used bits in the last byte (unused bits should be 0)
							e.g. if this is 6, then the bits used (x) in the last byte are: xxxxxx00,
							where MSb is the leftmost bit, LSb is the rightmost bit
	0x0D 	-       WORD 	Pause after this block (ms.)
	0x0F 	N       BYTE[3] Length of data that follow
	0x12 	-       BYTE[N] Data as in .TAP files

	This block is very similar to the normal TAP block but with some additional info on the timings
	and other important differences. The same tape encoding is used as for the standard speed data block.
	If a block should use some non-standard sync or pilot tones (i.e. all sorts of protection schemes)
	then use the next three blocks to describe it.

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

class TzxBlock11 : public TzxBlock
{
public:
	uint16  cc_pilot;       // 2168
	uint16  cc_sync1;       // 667
	uint16  cc_sync2;       // 735
	uint16  cc_bit0;        // 855
	uint16  cc_bit1;        // 1710
	uint16  ppulses;        // num pilot pulses: 8063 header (flag<128), 3223 data (flag>=128)
	uint8   lastbits;       // used bits in last byte
	uint16  pause_ms;       // pause after this block [ms]
	uint32  cnt;            // num. bytes that follow (3 bytes)
	u8ptr   data;

public:
		 TzxBlock11()				:TzxBlock(0x11,yes),data(NULL){}
		 TzxBlock11(TapData const&);
		~TzxBlock11()				{ delete[] data; }
	void read(FD& fd);
	void write(FD &fd)			const;
	void write(CswBuffer&)		const;
	bool is_end_block()			const { return pause_ms!=0; }
	cstr get_info()				const { return "Turbo speed data block"; }
	cstr get_info2()			const { return usingstr("%i bytes",cnt-2); }
};


void TzxBlock11::read(FD &fd)
{
	xlogIn("Block 0x11: turbo speed tape block");
	assert(data==NULL);

	cc_pilot = fd.read_uint16_z();
	cc_sync1 = fd.read_uint16_z();
	cc_sync2 = fd.read_uint16_z();
	cc_bit0  = fd.read_uint16_z();
	cc_bit1  = fd.read_uint16_z();
	ppulses  = fd.read_uint16_z();
	lastbits = fd.read_uint8();
	pause_ms = fd.read_uint16_z();
	cnt      = fd.read_uint24_z();
	data     = new uint8[cnt]; fd.read_bytes(data,cnt);

	if(lastbits>8) throw data_error("tzx block 0x11: last bits > 8");
	if(cnt==0)     throw data_error("tzx block 0x11: cnt == 0");

	xlogline("cc_pilot = %i",int(cc_pilot));
	xlogline("cc_sync1 = %i",int(cc_sync1));
	xlogline("cc_sync2 = %i",int(cc_sync2));
	xlogline("cc_bit0  = %i",int(cc_bit0));
	xlogline("cc_bit1  = %i",int(cc_bit1));
	xlogline("ppulses  = %i",int(ppulses));
	xlogline("lastbits = %i",int(lastbits));
	xlogline("pause_ms = %i",int(pause_ms));
	xlogline("data cnt = %i",int(cnt));
}

void TzxBlock11::write(FD& fd) const
{
	fd.write_uint16_z(cc_pilot);
	fd.write_uint16_z(cc_sync1);
	fd.write_uint16_z(cc_sync2);
	fd.write_uint16_z(cc_bit0);
	fd.write_uint16_z(cc_bit1);
	fd.write_uint16_z(ppulses);
	fd.write_uint8   (lastbits);
	fd.write_uint16_z(pause_ms);
	fd.write_uint24_z(cnt);
	fd.write_bytes   (data,cnt);
}

void TzxBlock11::write(CswBuffer& bu) const
{
	if(bu.getCurrentPhase()==(ppulses&1)) xlogline("tzx block 0x11: inverted signal");

	bu.writePureTone( ppulses, cc_pilot*SPCC );
	bu.writePulse   ( cc_sync1 * SPCC );
	bu.writePulse   ( cc_sync2 * SPCC );
	bu.writePureData( data, (cnt-1)*8+lastbits, cc_bit0*SPCC, cc_bit1*SPCC );
	bu.writeTzxPause( pause_ms*0.001 );
}


// -------------------------------------------------------------
// Block 0x12: Pure Tone

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Length of one pulse in T-states
	0x02 	-       WORD 	Number of pulses

	This will produce a tone which is basically the same as the pilot tone in the ID 10, ID 11 blocks.
	You can define how long the pulse is and how many pulses are in the tone.

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

struct TzxBlock12 : public TzxBlock
{
	uint16  cc_per_pulse;
	uint16  num_pulses;

	TzxBlock12()					:TzxBlock(0x12,yes){}
	TzxBlock12(uint16 cc, uint16 n):TzxBlock(0x12,yes),cc_per_pulse(cc),num_pulses(n){}
	void read(FD&);
	void write(FD&)				const;
	void write(CswBuffer&)		const;
};

void TzxBlock12::read(FD &fd)
{
	xlogIn("Block 0x12: pure tone");
	cc_per_pulse = fd.read_uint16_z();
	num_pulses   = fd.read_uint16_z();
}

void TzxBlock12::write(FD& fd) const
{
	fd.write_uint16_z(cc_per_pulse);
	fd.write_uint16_z(num_pulses);
}

void TzxBlock12::write(CswBuffer& bu) const
{
	bu.writePureTone(num_pulses,cc_per_pulse*SPCC);
}


// -------------------------------------------------------------
// Sequence of pulses of various lengths

/*  Offset 	Value 	Type 	Description
	0x00 	N       BYTE 	Number of pulses
	0x01 	-       WORD[N] Pulses' lengths

	This will produce N pulses, each having its own timing.
	Up to 255 pulses can be stored in this block; this is useful for
	non-standard sync tones used by some protection schemes.

	total chunk length = [00]*2+1

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

struct TzxBlock13 : public TzxBlock
{
	uint8	numpulses;		// num pulses
	uint16*	data;			// cc_per_pulse

	TzxBlock13()				:TzxBlock(0x13,yes),data(0){}
	~TzxBlock13()				{ delete[] data; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
};

void TzxBlock13::read(FD &fd)
{
	xlogIn("Block 0x13: pulses of various length");
	assert(data==NULL);

	numpulses = fd.read_uint8();
	data = new uint16[numpulses]; fd.read_data(data,numpulses);
}

void TzxBlock13::write(FD& fd) const
{
	fd.write_uint8(numpulses);
	fd.write_data(data,numpulses);
}

void TzxBlock13::write(CswBuffer& bu) const
{
	for(uint i=0;i<numpulses;i++) bu.writePulse( peek2Z(data+i) * SPCC );
}


// -------------------------------------------------------------
// Block14: Pure data block

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Length of ZERO bit pulse
	0x02 	-       WORD 	Length of ONE bit pulse
	0x04 	-       BYTE 	Used bits in last byte (unused bits should be 0)
							e.g. if this is 6, then the bits used (x) in the last byte are: xxxxxx00,
							where MSb is the leftmost bit, LSb is the rightmost bit
	0x05 	-       WORD 	Pause after this block (ms.)
	0x07 	N       BYTE[3] Length of data that follow
	0x0A 	-       BYTE[N] Data as in .TAP files

	This is the same as in the turbo loading data block, except that it has no pilot or sync pulses.

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

struct TzxBlock14 : public TzxBlock
{
	uint16  cc_bit0;        // 855
	uint16  cc_bit1;        // 1710
	uint8   lastbits;       // bits in last byte
	uint16  pause_ms;       // pause after this block [ms]
	uint32  cnt;            // num. bytes that follow (3 bytes)
	uint8*	data;

	TzxBlock14()			:TzxBlock(0x14,yes),data(0){}
	~TzxBlock14()			{ delete[] data; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const { return pause_ms!=0; }
	cstr get_info()			const { return "Pure data block [0x14]"; }
	cstr get_info2()		const { return usingstr("%i bytes",cnt-2); }
};

void TzxBlock14::read(FD &fd)
{
	xlogIn("Block 0x12: pure data block");
	assert(data==NULL);

	cc_bit0  = fd.read_uint16_z();
	cc_bit1  = fd.read_uint16_z();
	lastbits = fd.read_uint8   ();
	pause_ms = fd.read_uint16_z();
	cnt      = fd.read_uint24_z();
	data     = new uint8[cnt]; fd.read_bytes(data,cnt);

	if(lastbits>8) throw data_error("tzx block 0x14: last bits > 8");
	if(cnt==0)     throw data_error("tzx block 0x14: cnt == 0");
}

void TzxBlock14::write(FD& fd) const
{
	fd.write_uint16_z(cc_bit0);
	fd.write_uint16_z(cc_bit1);
	fd.write_uint8   (lastbits);
	fd.write_uint16_z(pause_ms);
	fd.write_uint24_z(cnt);
	fd.write_bytes   (data,cnt);
}

void TzxBlock14::write(CswBuffer& bu) const
{
	if(bu.getCurrentPhase()!=0) { xlogline("tzx block 0x14: inverted signal"); }

	bu.writePureData( data, (cnt-1)*8+lastbits, cc_bit0*SPCC, cc_bit1*SPCC );
	bu.writeTzxPause( pause_ms*0.001 );
}


// -------------------------------------------------------------
// Block 0x15: Direct recording block

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Number of T-states per sample (bit of data)
	0x02 	-       WORD 	Pause after this block in milliseconds (ms.)
	0x04 	-       BYTE 	Used bits (samples) in last byte of data (1-8)
							e.g. if this is 2, only first two samples of the last byte will be played
	0x05 	N       BYTE[3] Length of samples' data
	0x08 	-       BYTE[N] Samples data. Each bit represents a state on the EAR port (i.e. one sample).
							MSb is played first.

	This block is used for tapes which have some parts in a format such that the turbo loader block
	cannot be used. This is not like a VOC file, since the information is much more compact.
	Each sample value is represented by one bit only (0 for low, 1 for high) which means that
	the block will be at most 1/8 the size of the equivalent VOC.
	The preferred sampling frequencies are 22050 or 44100 Hz (158 or 79 T-states/sample).
	Please, if you can, don't use other sampling frequencies.
	Please use this block only if you cannot use any other block.

	total chunk length = [05,06,07]+8

	Zeros and ones in 'Direct recording' blocks mean low and high pulse levels respectively.
	The 'current pulse level' after playing a Direct Recording block or CSW recording block is the last level played.
*/

struct TzxBlock15 : public TzxBlock
{
	uint16  cc_per_sample;
	uint16  pause_ms;
	uint8   lastbits;   // used bits in last byte
	uint32  cnt;        // num. bytes that follow (3 bytes)
	uint8*	data;       // each bit represents one sample, as seen by the ear port

	TzxBlock15()				:TzxBlock(0x15,yes),data(0){}
	~TzxBlock15()				{ delete[] data; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const { return pause_ms!=0; }
};

void TzxBlock15::read(FD& fd)
{
	xlogIn("Block 0x15: direct recording (samples)");
	assert(!data);

	cc_per_sample = fd.read_uint16_z();
	pause_ms    = fd.read_uint16_z();
	lastbits    = fd.read_uint8();			// 1 .. 8
	cnt         = fd.read_uint24_z();		// ≥ 1
	data        = new uint8[cnt]; fd.read_bytes(data,cnt);

	if(cc_per_sample<8000) throw data_error("tzx block 0x15: sample rate too low: %u", uint(cc_per_sample));
	if(lastbits>8)	throw data_error("tzx block 0x15: last bits > 8");
	if(cnt==0)		throw data_error("tzx block 0x15: cnt = 0");
	if(lastbits==0) { lastbits=8; cnt--; }							  // just in case			 / for polarity
	if(cnt==0)		throw data_error("tzx block 0x15: num bits = 0"); // block must contain at least one sample
}

void TzxBlock15::write(FD& fd) const
{
	fd.write_uint16_z(cc_per_sample);
	fd.write_uint16_z(pause_ms);
	fd.write_uint8   (lastbits);
	fd.write_uint24_z(cnt);
	fd.write_bytes   (data,cnt);
}

void TzxBlock15::write(CswBuffer& bu) const
{
	bool bit = data[0]>>7;
	uint n = 0;   // samples_per_pulse counter
	uint i = 0;   // index in data
	uint j = 0;   // index in byte

	Time sps = cc_per_sample * SPCC;		// seconds per sample

	while(i<cnt-1 || ((i==cnt-1)&&j<lastbits))
	{
		bool b = (data[i] >> (7-j)) & 1;
		if(++j==8) { i++; j=0; }
		if(b==bit) n++;
		else { bu.writePulse(n*sps,bit); bit=b; n=1; }
	}
	bu.writePulse(n*sps,bit);

	if(pause_ms) bu.writeTzxPause(pause_ms * 0.001);
	else		 bu.writePulseCc(0);       // -> current level := not toggled
}


// -------------------------------------------------------------
// Block 0x18: CSW recording block

/*  Offset 	Value 	Type 	Description
	0x00 	10+N 	DWORD 	Block length (without these four bytes)
	0x04 	-       WORD 	Pause after this block (in ms).
	0x06 	-       BYTE[3] Sampling rate
	0x09 	-       BYTE 	Compression type
							0x01:   RLE
							0x02:   Z-RLE
	0x0A 	-       DWORD 	Number of stored pulses (after decompression, for validation purposes)
	0x0E 	-       BYTE[N] CSW data, encoded according to the CSW file format specification.

	This block contains a sequence of raw pulses encoded in CSW format v2 (Compressed Square Wave).
	For a complete description of the CSW compression format, see the CSW documentation.

	total chunk length = [00,01,02,03] + 4

	Zeros and ones in 'Direct recording' blocks mean low and high pulse levels respectively.
	The 'current pulse level' after playing a Direct Recording block or CSW recording block is the last level played.
*/

struct TzxBlock18 : public TzxBlock
{
	uint32	blen;           // block length (without these 4 bytes)
	uint16	pause_ms;       // pause after this block [ms]
	uint32	sam_per_sec;    // samples per second (3 bytes)
	uint8	compression;    // 0x01=RLE, 0x02=Z-compressed RLE
	uint32	num_pulses;     // Number of stored pulses: after decompression, for validation
	uint8*	data;           // CSW data, encoded according to the CSW file format specification.

	TzxBlock18()				:TzxBlock(0x18,yes),data(NULL){}
	TzxBlock18(CswBuffer const&, bool last_pulse_is_pause);
	~TzxBlock18()				{ delete[] data; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const	{ return pause_ms!=0; }
};

void TzxBlock18::read(FD &fd)
{
	xlogIn("Block 0x18: CSW recording");
	assert(!data);

	blen        = fd.read_uint32_z(); if(blen<=10) throw data_error("tzx block 0x18: count <= 0");
	pause_ms    = fd.read_uint16_z();
	sam_per_sec = fd.read_uint24_z();
	compression = fd.read_uint8   ();
	num_pulses  = fd.read_uint32_z();
	data        = new uint8[blen-10]; fd.read_bytes(data,blen-10);

	if(compression!=1&&compression!=2) throw data_error("tzx block 0x18: ill. compression mode");

	if(compression==2)  // Z-compressed data
	{                   // => uncompress bu[len] (compressed)  -->  bu[len] (uncompressed)

		uLongf zlen = num_pulses;		// note: data may contain x-long pulses which are 5 bytes long
	c:  zlen += zlen/2+1;
		u8ptr zbu = new uint8[zlen];

		int err = uncompress( zbu, &zlen, data, blen-10 );		// zlib.h

		switch(err)
		{
		case Z_OK:			// success
			delete[] data;
			data = zbu;
			compression = 1;
			blen = zlen+10;
			break;
		case Z_BUF_ERROR:	// dest[] too small
			delete[] zbu;
			goto c;
		case Z_MEM_ERROR:	throw bad_alloc();
		case Z_DATA_ERROR:	throw data_error("tzx block 0x18: corrupted data (uncompress)");
		default:			throw data_error("tzx block 0x18: unknown error (uncompress)");
		}
	}

	uint32 pulses = 0;

	for(uint32 i=0;i<blen-10;i++)
	{
		pulses++;
		if(data[i]==0)
		{
			if(i+4>blen-10) throw data_error("tzx block 0x18: corrupted data (final pulse)");
			i += 4;
		}
	}

	if(pulses != num_pulses) throw data_error("tzx block 0x18: corrupted data (num pulses)");
}

void TzxBlock18::write(FD& fd) const     // TODO: compress data before writing
{
	fd.write_uint32_z(blen);
	fd.write_uint16_z(pause_ms);
	fd.write_uint24_z(sam_per_sec);
	fd.write_uint8   (compression);
	fd.write_uint32_z(num_pulses);
	fd.write_bytes   (data,blen-10);
}

void TzxBlock18::write(CswBuffer& bu) const
{
	Time sps = 1.0/sam_per_sec;			// seconds per sample

	for(uint i=0;i<blen-10;i++)
	{
		if(data[i]) { bu.writePulse( data[i] * sps ); }
		else { bu.writePulse( peek4Z(data+(i+1)) * sps ); i += 4; }
	}

	if(pause_ms) bu.writeTzxPause( pause_ms * 0.001 );
	else		 bu.writePulseCc(0);	// last pulse level := don't toggle
}


// -------------------------------------------------------------
// Block 0x19: Generalized data block

/*  Offset 	Value        Type           Description
	0x00 	-            DWORD          Block length (without these four bytes)
	0x04 	-            WORD           Pause after this block (ms)

	0x06 	p_data_cnt 	 DWORD          Total number of symbols in pilot block (can be 0)
	0x0A 	p_symdef_max BYTE           Maximum number of pulses per pilot symbol
	0x0B 	p_symdef_cnt BYTE           Number of symbols in the pilot alphabet table (0=256)

	0x0C 	d_data_cnt 	 DWORD          Total number of symbols in data block (can be 0)
	0x10 	d_symdef_max BYTE           Maximum number of pulses per data symbol
	0x11 	d_symdef_cnt BYTE           Number of symbols in the data alphabet table (0=256)

	0x12 	-            SYMDEF[p_symdef_cnt]   Pilot and sync symbols definition table
												This field is present only if p_data_cnt>0
	var     -            PRLE[p_data_cnt]       Pilot data stream
												This field is present only if p_data_cnt>0
	var     -            SYMDEF[d_symdef_cnt]   Data symbols definition table
												This field is present only if d_data_cnt>0
	var     -            BYTE[num_databytes]    Data stream
												This field is present only if d_data_cnt>0

	This block can represent a wide range of d_data encoding techniques.
	The basic idea is that each loading component (pilot tone, sync pulses, d_data)
	is associated to a specific sequence of pulses, where each sequence (wave)
	can contain a different number of pulses from the others. In this way we can have
	a situation where bit 0 is represented with 4 pulses and bit 1 with 8 pulses.

	0x12 ff:
		only if p_data_cnt>0:
		{
			Symbol definition table:
			{
				byte    initial_phase,
				short   cc_per_pulse[p_symdef_max]
			}	symdef[p_symdef_cnt];

			Pilot data stream:
			{
				byte    symbol_idx,
				short   repetitions
			}	data[p_data_cnt];
		}
		only if d_data_cnt>0:
		{
			Symbol definition table:
			{
				byte    initial_phase,
				short   cc_per_pulse[d_data_max]
			}	symdef[d_symdef_cnt];

			Data d_data stream:
			{
				byte    bit_stream[]
			}   data[num_databytes];
		}

	total chunk length = [00,01,02,03] + 4

	The 'current pulse level' after playing the blocks ID 10,11,12,13,14 or 19 is the opposite
	of the last pulse level played, so that a subsequent pulse will produce an edge.
*/

struct SymDef       // Helper class: Symbol Definition
{
	uint    flags;  // bits 1…0: starting symbol polarity:
					//      00: opposite to the current level (make an edge, as usual) - default
					//      01: same as the current level (no edge - prolongs the previous pulse)
					//      10: force low level
					//      11: force high level
	uint    pulses; // pulses
	u16ptr  data;   // Array of pulse lengths; 0-delimited.  size = max+1

	SymDef():data(NULL){}

	void read   (FD& fd, uint max);
	void write  (FD& fd, uint max) const;
	void store_pulses(CswBuffer&) const;
	bool next_pulse(bool) const;
};

void SymDef::read(FD &fd, uint max)     // read from file
{
	assert(data==NULL);
	xlogIn("new Symbol:");

	flags  = fd.read_uint8();    xlogline("flags = %i",int(flags));
	pulses = 0;
	data   = new uint16[max+1]; memset(data,0,(max+1)*2);  // add final stopper if cnt==max
	uint n = 0;
	uint i = 0;
	xxlog("cc = ");
	while( i<max && (n=fd.read_uint16_z()) )
	{
		data[i++] = n;
		xxlog("%i ",int(data[i-1]));
	}
	xxlogNl();
	if(i==0) throw data_error("tzx block19 read symdef: pulses==0");
	pulses = i;
	if(i<max) fd.skip_bytes((max-i-1)*2);
}

void SymDef::write(FD &fd, uint max) const  // write to file
{
	fd.write_uint8(flags);
	for(uint i=0;i<max;i++) fd.write_uint16_z(data[i]);
}

void SymDef::store_pulses(CswBuffer& bu) const  // store pulses into CswBuffer
{
	uint16* d = data;

	switch(flags)
	{
	case 1:	bu.appendToPulse(*d++ * SPCC); break;
	case 2:
	case 3:	// if(bu.getCurrentPhase()!=(flags&1))
			// logline("store_pulse appends to pulse");
			bu.writePulse(*d++ * SPCC,flags&1); break;
	default: break;
	}

	while(*d) { bu.writePulse(*d++ * SPCC); }
}

// calc. the pulse level after this symbol:
bool SymDef::next_pulse(bool p) const
{
	uint16* d = data;
	switch(flags)
	{
	case 1:	d++; break;		// first pulse appends (don't toggle)
	case 2:	p = 0; break;	// first pulse := 0
	case 3:	p = 1; break;	// first pulse := 1
	default: break;			// first pulse toggles (default)
	}
	while(*d++) { p^=1; }
	return p;
}

struct TzxBlock19 : public TzxBlock  // Generalized d_data block
{
	uint32  blen;           // block length (without these 4 bytes)
	uint16  pause_ms;       // pause after this block [ms]

	uint32  p_data_cnt;     // Total number of symbols in pilot data block (can be 0)
	uint    p_symdef_max;   // Maximum number of pulses per pilot symbol
	uint    p_symdef_cnt;   // Number of symbols in the pilot alphabet table (0=256)

	uint32  d_data_cnt;     // Total number of symbols (~bits) in data stream (can be 0)
	uint    d_symdef_max;   // Maximum number of pulses per data symbol
	uint    d_symdef_cnt;   // Number of symbols in the data alphabet table (0=256)

	SymDef* p_symdef;       // if p_data_cnt>0: symdef[p_symdef_cnt]
	u8ptr   p_data;         // if p_data_cnt>0: PRLE[p_data_cnt] pilot data stream
	SymDef* d_symdef;       // if d_data_cnt>0: symdef[d_symdef_cnt]
	u8ptr   d_data;         // if d_data_cnt>0: data[num_databytes] pilot data stream

	uint    bits_per_symbol;// calculated: = ceil(log2(d_symdef_cnt));
	uint32  mask_databits;
	uint32  num_databits;   // calculated: = bits_per_symbol*d_data_cnt;
	uint32  num_databytes;  // calculated: = (databits+7)/8;

	TzxBlock19		()              :TzxBlock(0x19,yes),p_symdef(0),p_data(0),d_symdef(0),d_data(0){}
	TzxBlock19		(O80Data const&);
	~TzxBlock19	();
	void read	(FD& fd);
	void write	(FD &fd)	const;
	void write	(CswBuffer &bu) const;
	bool is_end_block()		const	{ return pause_ms!=0; }

	SymDef& get_p_symbol    (uint i) const  { return p_symdef[p_data[3*i]]; }
	uint16  get_p_repeat    (uint i) const  { return peek2Z(p_data+(3*i+1)); }

	SymDef& get_d_symbol    (uint32 i) const
	{
		uint32 idx = (i*bits_per_symbol)/8;
		uint shift = 16 - bits_per_symbol - (i*bits_per_symbol)%8;
		return d_symdef[ (peek2X(d_data+idx) >> shift) & mask_databits ];
	}
};


TzxBlock19::~TzxBlock19()
{
	if(p_symdef) for(uint i=0;i<p_symdef_cnt;i++) delete[] p_symdef[i].data;
	if(d_symdef) for(uint i=0;i<d_symdef_cnt;i++) delete[] d_symdef[i].data;
	delete[] p_symdef;
	delete[] p_data;
	delete[] d_symdef;
	delete[] d_data;
}

void TzxBlock19::read(FD &fd)
{
	xlogIn("Block 0x19: generalized data block");
	assert(p_symdef==NULL);
	assert(p_data==NULL);
	assert(d_symdef==NULL);
	assert(d_data==NULL);

	blen         = fd.read_uint32_z();   uint32 bend = fd.file_position()+blen;
	pause_ms     = fd.read_uint16_z();
	p_data_cnt   = fd.read_uint32_z();
	p_symdef_max = fd.read_uint8   ();   if(p_symdef_max==0) p_symdef_max=256;
	p_symdef_cnt = fd.read_uint8   ();   if(p_symdef_cnt==0) p_symdef_cnt=256;
	d_data_cnt   = fd.read_uint32_z();
	d_symdef_max = fd.read_uint8   ();   if(d_symdef_max==0) d_symdef_max=256;
	d_symdef_cnt = fd.read_uint8   ();   if(d_symdef_cnt==0) d_symdef_cnt=256;

	xlogline("blen = %i",blen);
	xlogline("pause_ms = %i",pause_ms);
	xlogline("p_data_cnt = %i",p_data_cnt);
	xlogline("p_symdef_max = %i",p_symdef_max);
	xlogline("p_symdef_cnt = %i",p_symdef_cnt);
	xlogline("d_data_cnt = %i",d_data_cnt);
	xlogline("d_symdef_max = %i",d_symdef_max);
	xlogline("d_symdef_cnt = %i",d_symdef_cnt);

	if(p_data_cnt)
	{
		p_symdef = new SymDef[p_symdef_cnt];
		for(uint i=0;i<p_symdef_cnt;i++) { p_symdef[i].read(fd,p_symdef_max); }

		p_data = new uint8[p_data_cnt*3];
		fd.read_bytes(p_data,p_data_cnt*3);
	}

	if(d_data_cnt)
	{
		bits_per_symbol = ceil(log2(d_symdef_cnt));
		num_databits    = bits_per_symbol*d_data_cnt;
		num_databytes   = (num_databits+7)/8;
		mask_databits   = (1<<bits_per_symbol)-1;

		d_symdef = new SymDef[d_symdef_cnt];
		for(uint i=0;i<d_symdef_cnt;i++) { d_symdef[i].read(fd,d_symdef_max); }

		d_data = new uint8[num_databytes+1];    // +1 wg. Wort-Zugriff in get_d_symbol()
		fd.read_bytes(d_data,num_databytes);
	}

	assert(fd.file_position()==bend);
}

void TzxBlock19::write(FD& fd) const
{
	 fd.write_uint32_z(blen);
	 fd.write_uint16_z(pause_ms);
	 fd.write_uint32_z(p_data_cnt);
	 fd.write_uint8   (p_symdef_max);
	 fd.write_uint8   (p_symdef_cnt);
	 fd.write_uint32_z(d_data_cnt);
	 fd.write_uint8   (d_symdef_max);
	 fd.write_uint8   (d_symdef_cnt);

	 if(p_data_cnt)
	 {
		for(uint i=0;i<p_symdef_cnt;i++) { p_symdef[i].write(fd,p_symdef_max); }
		fd.write_bytes(p_data,p_data_cnt*3);
	 }

	 if(d_data_cnt)
	 {
		for(uint i=0;i<d_symdef_cnt;i++) { d_symdef[i].write(fd,d_symdef_max); }
		fd.write_bytes(d_data,num_databytes);
	 }
}

void TzxBlock19::write(CswBuffer& bu) const
{
	// write pilot&sync:
	for(uint32 i=0;i<p_data_cnt;i++)
		for( uint r=get_p_repeat(i);r;r-- )
			get_p_symbol(i).store_pulses(bu);

	// write data:
	for(uint32 i=0;i<d_data_cnt;i++)
		get_d_symbol(i).store_pulses(bu);

	// write pause (if any):
	bu.writeTzxPause( pause_ms * 0.001 );
}


// -------------------------------------------------------------
// Block 0x20: Pause (silence) or 'Stop the tape' command

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Pause duration (ms.)

	This will make a silence (low amplitude level (0)) for a given time in milliseconds.
	If the value is 0 then the emulator or utility should STOP THE TAPE.

	total chunk length = 2
*/

struct TzxBlock20 : public TzxBlock
{
	uint16      pause_ms;

	TzxBlock20()				:TzxBlock(0x20,yes),pause_ms(0){}
	TzxBlock20(uint16 ms)		:TzxBlock(0x20,yes),pause_ms(ms){}
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const { return yes; }
};

void TzxBlock20::read(FD &fd)
{
	xlogIn("Block 0x20: pause");
	pause_ms = fd.read_uint16_z();
}

void TzxBlock20::write(FD& fd) const
{
	fd.write_uint16_z(pause_ms);
}

void TzxBlock20::write(CswBuffer& bu) const
{
	bu.writeTzxPause(pause_ms ? pause_ms*0.001 : 5.0);
}


// -------------------------------------------------------------
// Block 0x21: Group start

/*  Offset 	Value 	Type 	Description
	0x00 	L       BYTE 	Length of the group name string
	0x01 	-       CHAR[L] Group name in ASCII format (please keep it under 30 characters long)

	This block marks the start of a group of blocks which are to be treated as one single (composite) block.
	This is handy for tapes that use lots of subblocks like Bleepload, which may have over 160 sub blocks.
	You can also give the group a name (example 'Bleepload Block 1').

	For each group start block, there must be a group end block. Nesting of groups is not allowed.

	total chunk length = [00]+01
*/

class TzxBlock21 : public TzxBlock
{
	uint8   len;		// name len
	str     text;		// name
	TzxBlock* blocks;	// contained blocks, incl. EndOfGroup block

public:
	TzxBlock21()				:TzxBlock(0x21,yes),text(NULL),blocks(NULL){}
	~TzxBlock21()				{ delete[]text; delete blocks; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const;
	cstr get_info()			const { return text; }
};


// endet dieser Block mit "Pause"?
bool TzxBlock21::is_end_block() const
{
	bool f = no;
	for(TzxBlock* p = blocks; p; p=p->next)
	{
		if(p->has_data) f = p->is_end_block();
	}
	return f;
}

void TzxBlock21::read(FD &fd)
{
	xlogIn("Block 0x21: group start");
	assert(!text);
	assert(!blocks);

	len  = fd.read_uint8();
	text = newstr(len); fd.read_bytes(text,len);
	for(uint i=0;i<len;i++) { if(text[i]<' ') text[i]=' '; }
	// TODO: convert to utf-8

	blocks = readFromFile(fd,0x22/*GroupEnd*/);
	if(!blocks || blocks->last()->id!=0x22) throw data_error("tzx block 0x21: block 0x22 EndGroup missing");
}

void TzxBlock21::write(FD& fd) const
{
	fd.write_uint8(len);
	fd.write_bytes(text,len);

	if(blocks) blocks->writeToFile(fd);
}

void TzxBlock21::write(CswBuffer& bu) const
{
	if(blocks) blocks->storeCsw(bu);
}


// -------------------------------------------------------------
// Block 0x22: Group end

/*  This indicates the end of a group. This block has no body.
*/

struct TzxBlock22 : public TzxBlock
{
	TzxBlock22()				:TzxBlock(0x22,no){}
	void read(FD&)			{ xlogline("Block 22: group end"); }
	void write(FD&)			const {}
};


// -------------------------------------------------------------
// Block 0x24: Loop start

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Number of repetitions (greater than 1)

	If you have a sequence of identical blocks, or of identical groups of blocks,
	you can use this block to tell how many times they should be repeated.
	For simplicity reasons don't nest loop blocks!

	total chunk length = 2
*/

class TzxBlock24 : public TzxBlock
{
	uint16      cnt;				// repetitions > 1
	TzxBlock*	blocks;				// contained blocks, incl. EndOfLoop block

public:
		 TzxBlock24()			:TzxBlock(0x24,yes),blocks(NULL){}
		 ~TzxBlock24()			{ delete blocks; }
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const;
};

// endet dieser Block mit "Pause"?
bool TzxBlock24::is_end_block() const
{
	bool f = no;
	for(TzxBlock* p = blocks; p; p=p->next)
	{
		if(p->has_data) f = p->is_end_block();
	}
	return f;
}

void TzxBlock24::read(FD &fd)
{
	xlogIn("Block 0x24: loop start");
	assert(!blocks);

	cnt = fd.read_uint16_z();
	blocks = readFromFile(fd,0x25/*LoopEnd*/);
	if(!blocks || blocks->last()->id!=0x25) throw data_error("tzx block 0x24: block 0x25 LoopEnd missing");
}

void TzxBlock24::write(FD& fd) const
{
	fd.write_uint16_z(cnt);
	if(blocks) blocks->writeToFile(fd);
}

void TzxBlock24::write(CswBuffer& bu) const
{
	if(blocks) for(uint r=0; r<cnt; r++) blocks->storeCsw(bu);
}


// -------------------------------------------------------------
// Block 0x25: Loop end

struct TzxBlock25 : public TzxBlock
{
	TzxBlock25()			:TzxBlock(0x25,no){}
	void read(FD&)		{ xlogline("Block 25: loop end"); }
	void write(FD&)		const {}
};


// -------------------------------------------------------------
// Block 0x2A: Stop the tape if in 48K mode

/*  Offset 	Value 	Type 	Description
	0x00 	0       DWORD 	Length of the block without these four bytes (0)

	When this block is encountered, the tape will stop ONLY if the machine is an 48K Spectrum.
	This block is to be used for multiloading games that load one level at a time in 48K mode,
	but load the entire tape at once if in 128K mode.

	This block has no body of its own, but follows the extension rule.

	TODO: diese Info nach oben durchreichen. zB. mit majorBlockInfo ?

	total chunk length = 4
*/

struct TzxBlock2A : public TzxBlock
{
	uint32  len;

	TzxBlock2A()				:TzxBlock(0x2A,yes){}
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
	bool is_end_block()		const { return yes; }
	cstr get_info()			const { return "Stop the tape if in 48k mode"; }
};

void TzxBlock2A::read(FD& fd)
{
	xlogIn("Block 0x2A: stop the tape in 48k mode");
	len = fd.read_uint32_z();
	if(len!=0) throw data_error("tzx block 0x2A: wrong len");
}

void TzxBlock2A::write(FD& fd) const
{
	fd.write_uint32_z(len);
}

void TzxBlock2A::write(CswBuffer& bu) const
{
	bu.writeTzxPause(5.0);
}


// -------------------------------------------------------------
// Block 0x2B: Set signal level

/*  Offset 	Value 	Type 	Description
	0x00 	1       DWORD 	Block length (without these four bytes)
	0x04 	-       BYTE 	Signal level (0=low, 1=high)

	This block sets the current signal level to the specified value (high or low).
	It should be used whenever it is necessary to avoid any ambiguities,
	e.g. with custom loaders which are level-sensitive.

	total chunk length = [00,01,02,03] + 1  =  5
*/

struct TzxBlock2B : public TzxBlock
{
	uint32  len;    // 1
	uint8   phase;  // 0 or 1

	TzxBlock2B()					:TzxBlock(0x2B,yes){}
	TzxBlock2B(bool p)				:TzxBlock(0x2B,yes),len(1),phase(p){}
	void read(FD&);
	void write(FD&)			const;
	void write(CswBuffer&)	const;
};

void TzxBlock2B::read(FD& fd)
{
	xlogIn("Block 0x2B: set signal level");

	len = fd.read_uint32_z();
	if(len!=1) throw data_error("tzx block 0x2B: wrong len");
	phase = fd.read_uint8() & 1;
}

void TzxBlock2B::write(FD& fd) const
{
	fd.write_uint32_z(len);
	fd.write_uint8(phase);
}

void TzxBlock2B::write(CswBuffer& bu) const
{
	bu.setPhase(phase);
}


// -------------------------------------------------------------
// Block 0x30: Text description

/*	Offset 	Value 	Type 	Description
	0x00 	N       BYTE    Length of the text description
	0x01 	-       CHAR[N] Text description in ASCII format

	This is meant to identify parts of the tape, so you know where level 1 starts,
	where to rewind to when the game ends, etc.
	This description is not guaranteed to be shown while the tape is playing,
	but can be read while browsing the tape or changing the tape pointer.

	The description can be up to 255 characters long but please keep it down to about 30
	so the programs can show it in one line (where this is appropriate).

	Please use 'Archive Info' block for title, authors, publisher, etc.

	total chunk length: [00]+01
*/

struct TzxBlock30 : public TzxBlock
{
	uint8   len;
	str     text;

	TzxBlock30()               :TzxBlock(0x30,no),text(NULL){}
	~TzxBlock30()              { delete[]text; }
	void read(FD&);
	void write(FD&)			const;
	cstr get_info()			const { return text; }
};

void TzxBlock30::read(FD& fd)
{
	xlogIn("Block 0x30: textual description");
	assert(!text);

	len = fd.read_uint8();
	text = newstr(len); fd.read_bytes(text,len);
	for(uint i=0;i<len;i++) { if(text[i]<' ') text[i]=' '; }
	// TODO: convert to utf-8
}

void TzxBlock30::write(FD& fd) const
{
	fd.write_uint8(len);
	fd.write_bytes(text,len);
}


// -------------------------------------------------------------
// Block 0x31: Message block  ((~subtitle))

/*	Offset 	Value 	Type 	Description
	0x00 	-       BYTE 	Time (in seconds) for which the message should be displayed
	0x01 	N       BYTE 	Length of the text message
	0x02 	-       CHAR[N] Message that should be displayed in ASCII format

	This will enable the emulators to display a message for a given time.
	This should not stop the tape and it should not make silence.
	If the time is 0 then the emulator should wait for the user to press a key.

	The text message should:
		stick to a maximum of 30 chars per line;
		use single 0x0D (13 decimal) to separate lines;
		stick to a maximum of 8 lines.

	If you do not obey these rules, emulators may display your message in any way they like.

	total chunk length = [01]+2
*/

struct TzxBlock31 : public TzxBlock
{
	uint8   time;       // time to display [sec]
	uint8   len;
	str     text;

	TzxBlock31()				:TzxBlock(0x31,no),text(NULL){}
	~TzxBlock31()				{ delete[] text; }
	void read(FD&);
	void write(FD&)			const;
	cstr get_info()			const { return text; }    // should only be used as a last resort
};

void TzxBlock31::read(FD& fd)
{
	xlogIn("Block 0x31: message");
	assert(!text);

	time = fd.read_uint8();
	len  = fd.read_uint8();
	text = newstr(len); fd.read_bytes(text,len);
	for(uint i=0;i<len;i++) { if(text[i]<' ') text[i]=' '; }	// TODO: 0x0D in msg: line break
	// TODO: convert to utf-8
}

void TzxBlock31::write(FD& fd) const
{
	fd.write_uint8(time);
	fd.write_uint8(len);
	fd.write_bytes(text,len);
}


// -------------------------------------------------------------
// Block 0x32: Archive info

/*  Offset 	Value 	Type 	Description
	0x00 	-       WORD 	Length of the whole block (without these two bytes)
	0x02 	N       BYTE 	Number of text strings
	0x03 	-       TEXT[N] List of text strings


	TEXT structure format
	Offset 	Value 	Type 	Description
	0x00 	-       BYTE 	Text identification byte:
							00 - Full title
							01 - Software house/publisher
							02 - Author(s)
							03 - Year of publication
							04 - Language
							05 - Game/utility type
							06 - Price
							07 - Protection scheme/loader
							08 - Origin
							FF - Comment(s)
	0x01 	L       BYTE 	Length of text string
	0x02 	-       CHAR[L] Text string in ASCII format

	Use this block at the beginning of the tape to identify the title of the game, author,
	publisher, year of publication, price (including the currency),
	type of software (arcade adventure, puzzle, word processor, ...),
	protection scheme it uses (Speedlock 1, Alkatraz, ...)
	and its origin (Original, Budget re-release, ...), etc.
	This block is built in a way that allows easy future expansion.
	The block consists of a series of text strings.
	Each text has its identification number (which tells us what the text means)
	and then the ASCII text. To make it possible to skip this block, if needed,
	the length of the whole block is at the beginning of it.

	If all texts on the tape are in English language then you don't have to supply the 'Language' field.

	The information about what hardware the tape uses is in the 'Hardware Type' block, so no need for it here.
*/

struct TzxBlock32 : public TzxBlock
{
	uint16  blen;   // length of block (w/o these 2 bytes)
	uint8   cnt;    // number of text strings
	str*    texte;  // array of texts
					//  text[0]=id
					//  text[1]=len
					//  text[2++]=text

	TzxBlock32()			:TzxBlock(0x32,no),cnt(0),texte(NULL){}
	~TzxBlock32()			{ if(texte) for(int i=0;i<cnt;i++) { delete[] texte[i]; } delete[] texte; }
	void read(FD&);
	void write(FD&)		const;
};

void TzxBlock32::read(FD& fd)
{
	xlogIn("Block 0x32: archive info");
	assert(!texte);

	blen = fd.read_uint16_z();
	cnt  = fd.read_uint8();
	texte = new str[cnt+1]; memset(texte,0,sizeof(str)*(cnt+1));

	uint n = 1;

	for(uint i=0;i<cnt;i++)
	{
		uint8 idf = fd.read_uint8();
		uint8 len = fd.read_uint8();
		texte[i]  = newstr(len+2);
		texte[i][0] = idf;
		texte[i][1] = len;
		fd.read_bytes(texte[i]+2,len);
		for(uint j=0;j<len;j++) { if(texte[i][j+2]<' ') texte[i][j+2]=' '; }
		// TODO: convert to utf-8
		n += 2+len;
	}

	if(n!=blen) throw data_error("tzx block 0x32: corrupted archive info block");
}

void TzxBlock32::write(FD& fd) const
{
	fd.write_uint16_z(blen);
	fd.write_uint8(cnt);
	for(uint i=0;i<cnt;i++) { fd.write_data(texte[i],texte[i][1]+2); }
}


// -------------------------------------------------------------
// Block 0x33: Hardware type

/*	Offset 	Value 	Type 	Description
	0x00 	N       BYTE 	Number of machines and hardware types for which info is supplied
	0x01 	-       HWINFO[N] 	List of machines and hardware

	HWINFO structure format
	Offset 	Value 	Type 	Description
	0x00 	-       BYTE 	Hardware type
	0x01 	-       BYTE 	Hardware ID
	0x02 	-       BYTE 	Hardware information:
							00 - The tape RUNS on this machine or with this hardware,
								  but may or may not use the hardware or special features of the machine.
							01 - The tape USES the hardware or special features of the machine,
								  such as extra memory or a sound chip.
							02 - The tape RUNS but it DOESN'T use the hardware
								  or special features of the machine.
							03 - The tape DOESN'T RUN on this machine or with this hardware.

	This blocks contains information about the hardware that the programs on this tape use.
	Please include only machines and hardware for which you are 100% sure that it either runs (or doesn't run) on
	or with, or you know it uses (or doesn't use) the hardware or special features of that machine.

	If the tape runs only on the ZX81 (and TS1000, etc.) then it clearly won't work on any Spectrum or
	Spectrum variant, so there's no need to list this information.

	If you are not sure or you haven't tested a tape on some particular machine/hardware combination
	then do not include it in the list.

	Hardware type 	Hardware ID
	00 - Computers 	00 - ZX Spectrum 16k
					01 - ZX Spectrum 48k, Plus
					02 - ZX Spectrum 48k ISSUE 1
					03 - ZX Spectrum 128k +(Sinclair)
					04 - ZX Spectrum 128k +2 (grey case)
					05 - ZX Spectrum 128k +2A, +3
					06 - Timex Sinclair TC-2048
					07 - Timex Sinclair TS-2068
					08 - Pentagon 128
					09 - Sam Coupe
					0A - Didaktik M
					0B - Didaktik Gama
					0C - ZX-80
					0D - ZX-81
					0E - ZX Spectrum 128k, Spanish version
					0F - ZX Spectrum, Arabic version
					10 - Microdigital TK 90-X
					11 - Microdigital TK 95
					12 - Byte
					13 - Elwro 800-3
					14 - ZS Scorpion 256
					15 - Amstrad CPC 464
					16 - Amstrad CPC 664
					17 - Amstrad CPC 6128
					18 - Amstrad CPC 464+
					19 - Amstrad CPC 6128+
					1A - Jupiter ACE
					1B - Enterprise
					1C - Commodore 64
					1D - Commodore 128
					1E - Inves Spectrum+
					1F - Profi
					20 - GrandRomMax
					21 - Kay 1024
					22 - Ice Felix HC 91
					23 - Ice Felix HC 2000
					24 - Amaterske RADIO Mistrum
					25 - Quorum 128
					26 - MicroART ATM
					27 - MicroART ATM Turbo 2
					28 - Chrome
					29 - ZX Badaloc
					2A - TS-1500
					2B - Lambda
					2C - TK-65
					2D - ZX-97
	01 - External storage
					00 - ZX Microdrive
					01 - Opus Discovery
					02 - MGT Disciple
					03 - MGT Plus-D
					04 - Rotronics Wafadrive
					05 - TR-DOS (BetaDisk)
					06 - Byte Drive
					07 - Watsford
					08 - FIZ
					09 - Radofin
					0A - Didaktik disk drives
					0B - BS-DOS (MB-02)
					0C - ZX Spectrum +3 disk drive
					0D - JLO (Oliger) disk interface
					0E - Timex FDD3000
					0F - Zebra disk drive
					10 - Ramex Millenia
					11 - Larken
					12 - Kempston disk interface
					13 - Sandy
					14 - ZX Spectrum +3e hard disk
					15 - ZXATASP
					16 - DivIDE
					17 - ZXCF
	02 - ROM/RAM type add-ons
					00 - Sam Ram
					01 - Multiface ONE
					02 - Multiface 128k
					03 - Multiface +3
					04 - MultiPrint
					05 - MB-02 ROM/RAM expansion
					06 - SoftROM
					07 - 1k
					08 - 16k
					09 - 48k
					0A - Memory in 8-16k used
	03 - Sound devices
					00 - Classic AY hardware (compatible with 128k ZXs)
					01 - Fuller Box AY sound hardware
					02 - Currah microSpeech
					03 - SpecDrum
					04 - AY ACB stereo (A+C=left, B+C=right); Melodik
					05 - AY ABC stereo (A+B=left, B+C=right)
					06 - RAM Music Machine
					07 - Covox
					08 - General Sound
					09 - Intec Electronics Digital Interface B8001
					0A - Zon-X AY
					0B - QuickSilva AY
					0C - Jupiter ACE
	04 - Joysticks 	00 - Kempston
					01 - Cursor, Protek, AGF
					02 - Sinclair 2 Left (12345)
					03 - Sinclair 1 Right (67890)
					04 - Fuller
	05 - Mice       00 - AMX mouse
					01 - Kempston mouse
	06 - Other controllers
					00 - Trickstick
					01 - ZX Light Gun
					02 - Zebra Graphics Tablet
					03 - Defender Light Gun
	07 - Serial ports
					00 - ZX Interface 1
					01 - ZX Spectrum 128k
	08 - Parallel ports
					00 - Kempston S
					01 - Kempston E
					02 - ZX Spectrum +3
					03 - Tasman
					04 - DK'Tronics
					05 - Hilderbay
					06 - INES Printerface
					07 - ZX LPrint Interface 3
					08 - MultiPrint
					09 - Opus Discovery
					0A - Standard 8255 chip with ports 31,63,95
	09 - Printers 	00 - ZX Printer, Alphacom 32 & compatibles
					01 - Generic printer
					02 - EPSON compatible
	0A - Modems 	00 - Prism VTX 5000
					01 - T/S 2050 or Westridge 2050
	0B - Digitizers 00 - RD Digital Tracer
					01 - DK'Tronics Light Pen
					02 - British MicroGraph Pad
					03 - Romantic Robot Videoface
	0C - Network adapters
					00 - ZX Interface 1
	0D - Keyboards & keypads
					00 - Keypad for ZX Spectrum 128k
	0E - AD/DA converters
					00 - Harley Systems ADC 8.2
					01 - Blackboard Electronics
	0F - EPROM programmers
					00 - Orme Electronics
					10 - Graphics 	00 - WRX Hi-Res
					01 - G007
					02 - Memotech
					03 - Lambda Colour
*/

struct TzxBlock33 : public TzxBlock
{
	uint8   cnt;
	u8ptr   data;   // data[3i+0]=hw_type
					// data[3i+1]=hw_id
					// data[3i+2]=hw_info

	TzxBlock33()				:TzxBlock(0x33,no),data(NULL){}
	~TzxBlock33()				{ delete[] data; }
	void read(FD&);
	void write(FD&)			const;
};

void TzxBlock33::read(FD& fd)
{
	xlogIn("Block 0x33: hardware type");
	assert(!data);

	cnt = fd.read_uint8();
	data = new uint8[cnt*3]; fd.read_bytes(data,cnt*3);
}

void TzxBlock33::write(FD& fd) const
{
	fd.write_uint8(cnt);
	fd.write_bytes(data,cnt*3);
}


// -------------------------------------------------------------
//          public TzxBlock methods:
// -------------------------------------------------------------


/*  read next tzx block from tape:
	returns NULL at eof
*/
//static
TzxBlock* TzxBlock::read_next(FD &fd) throws // data_error, file_error
{
	xlogIn("TzxBlock.read_next()");

	TzxBlock* block;

a:  if(fd.file_remaining()==0) return NULL;

	uint8 id = fd.read_uint8();
	xlogline("load tzx 0x%2x block",int(id));
	switch(id)
	{
	case 0x10:  block = new TzxBlock10; break;
	case 0x11:  block = new TzxBlock11; break;
	case 0x12:  block = new TzxBlock12; break;
	case 0x13:  block = new TzxBlock13; break;
	case 0x14:  block = new TzxBlock14; break;
	case 0x15:  block = new TzxBlock15; break;
	case 0x16:	throw data_error("unsupported tzx block: 0x16 C64_ROM_TAPE_DATA_BLOCK");    // deprecated
	case 0x17:	throw data_error("unsupported tzx block: 0x17 C64_TURBO_TAPE_DATA_BLOCK");  // deprecated
	case 0x18:  block = new TzxBlock18; break;
	case 0x19:  block = new TzxBlock19; break;
	case 0x20:  block = new TzxBlock20; break;
	case 0x21:  block = new TzxBlock21; break;
	case 0x22:  block = new TzxBlock22; break;
	case 0x23:  throw data_error("unsupported tzx block: 0x23 JUMP");   // tape probably won't load
	case 0x24:  block = new TzxBlock24; break;
	case 0x25:  block = new TzxBlock25; break;
	case 0x26:  throw data_error("unsupported tzx block: 0x26 CALL");   // tape probably won't load
	case 0x27:  throw data_error("unsupported tzx block: 0x27 RETURN"); // tape probably won't load
	case 0x28:  xlogline("readTzxBlock: skipping block 0x28 SELECT_BLOCK");       // ignore
				// not supported: writing back to tzx file will result in a corrupted SELECT block,
				// due to added and removed blocks => remove this block
				fd.skip_bytes(fd.read_uint16_z()); goto a;
	case 0x2A:  block = new TzxBlock2A; break;
	case 0x2B:  block = new TzxBlock2B; break;
	case 0x30:  block = new TzxBlock30; break;
	case 0x31:  block = new TzxBlock31; break;
	case 0x32:  block = new TzxBlock32; break;
	case 0x33:  block = new TzxBlock33; break;
	case 0x34:  // EMULATION INFO: deprecated in tzx 1.20
				xlogline("readTzxBlock: skipping block 0x34 EMULATION_INFO");       // ignore
				fd.skip_bytes(8); goto a;
	case 0x35:  // CUSTOM_INFO_BLOCK deprecated in tzx 1.20
				xlogline("readTzxBlock: skipping block 0x35 CUSTOM_INFO_BLOCK");    // ignore
				fd.skip_bytes(16); fd.skip_bytes(fd.read_uint32_z()); goto a;
	case 0x40:  throw data_error("unsupported tzx block: 0x40 SNAPSHOT");   // deprecated in tzx 1.20
	case 'Z':   fd.skip_bytes(9); goto a;                                   // concatenated files
	default:    if(id>=0x10&&id<0x60) { fd.skip_bytes(fd.read_uint32_z()); return NULL; }  // skip tzx 1.30++ block
				throw data_error("ill. tzx block number: 0x%02x",uint(id));
	}

	block->read(fd);

	if(id==0x30 && eq(lowerstr(leftstr(((TzxBlock30*)block)->text,8)),"created ")) // "Created with bla bla"
	{ delete block; goto a; }

	return block;
}



// -------------------------------------------------------------
//          Conversion methods:
// -------------------------------------------------------------


/*	convert TapData to Block10: standard tape block
	• the additional final pulse of a Jupiter Ace block might be fixed by the caller
	• POLARITY of this block depends on "next level" after prev. block
	  which should be 0 (as after TzxPause)
	  if caller want's to ensure standard polarity (pulse pairs 1-0)
		then caller may add one pilot pulse to toggle polarity
	• "next level" after this block is 0 (as after TzxPause)
*/
TzxBlock10::TzxBlock10(TapData const& q)
:
	TzxBlock(0x10,yes),
	data(NULL)
{
	pause_ms = uint16(minmax(1.0, q.pause*1000+0.5, 10000.0));
	cnt		 = q.count();
	data	 = new uint8[cnt]; memcpy(data, q.getData(), cnt);
}


/*	convert TapData to Block11: turbo speed tape block
	• timings are set as for ZX Spectrum (default) or Jupiter Ace (if is_jupiter=yes)
	• the additional final pulse of a Jupiter Ace block should be fixed by the caller
	• POLARITY of this block depends on "next level" after prev. block
	  if caller want's to ensure standard polarity (pulse pairs 1-0)
		then caller may add one pilot pulse to toggle polarity or prepend a Block2B 'set polarity'
	• "next level" after this block is 0 (as after TzxPause)
*/
TzxBlock11::TzxBlock11(TapData const& q)
:
	TzxBlock(0x11,yes),
	data(NULL)
{
	ppulses  = q.pilot_pulses;
	lastbits = 8;
	pause_ms = uint16(minmax(1.0, q.pause*1000+0.5, 10000.0));
	cnt		 = q.count();
	data	 = new uint8[cnt]; memcpy(data, q.getData(), cnt);

	if(q.isJupiter())
	{
		cc_pilot = cc_jup_pilot;	// 2011 @ 3.25 MHz
		cc_sync1 = cc_jup_sync1;	// 601
		cc_sync2 = cc_jup_sync2;	// 791
		cc_bit0  = cc_jup_bit0;		// 1:795,  0:801
		cc_bit1  = cc_jup_bit1;		// 1:1585, 0:1591
	}
	else
	{
		cc_pilot = cc_zxsp_pilot;	// 2168 @ 3.5 MHz
		cc_sync1 = cc_zxsp_sync1;	// 667
		cc_sync2 = cc_zxsp_sync2;	// 735
		cc_bit0  = cc_zxsp_bit0;	// 855
		cc_bit1  = cc_zxsp_bit1;	// 1710
	}
}


/*	convert a CswBuffer to Block18 'CSW data'
	• POLARITY of this block depends on "next phase" after the previous block
	  if caller want's to ensure correct polarity then caller may prepend a Block2B 'set polarity'
	• CALLER MUST CALL CswBuffer::normalize() before calling this c'tor,
	  because there may be zero-length pulses at any position in a CswBuffer, which isn't handled!
	• if last_pulse_is_pause=yes, then the last pulse is replaced by pause (min. 1ms, max. 10s) else pause=0
	• if last_pulse_is_pause=no,  then "next phase" after this block is the last phase played
	  which is same as own first phase if num. pulses is ODD
	• sample frequency is reduced to sps/16 so that most pulses will fit in one byte
	• currently the block is not Z-compressed		TODO
	• if resulting num_pulses=0, either because the CswBuffer was empty or
	  because it only contained one pulse and last_pulse_is_pause was set,
	  then this is an illegal condition which must be handled by the caller:
		bu.count=1 & last_pulse_is_pause=1 => pause_ms>0:  replace with Block20 'pause'
		bu.count=0 & last_pulse_is_pause=1 => pause_ms=0:  omit block or replace with Block20 'pause'
		bu.count=0 & last_pulse_is_pause=0 => pause_ms=0:  omit block or toggle polarity with Block2B 'set polarity'
*/
TzxBlock18::TzxBlock18(CswBuffer const& bu, bool last_pulse_is_pause)
:
	TzxBlock(0x18,yes),
	blen(10+bu.getTotalPulses()),		  // block length (without these 4 bytes)
	pause_ms(0),						  // no pause after this block
	sam_per_sec((bu.ccPerSecond()+8)>>4), // sps/16
	compression(1),						  // 0x01=RLE, 0x02=Z-compressed RLE
	num_pulses(bu.getTotalPulses()),	  // Number of stored pulses: after decompression, for validation
	data(NULL)
{
	xlogIn("TzxBlock: new Block18(CswBuffer)");

	assert(bu.is_normalized());

	uint16 const* q = bu.getData();
	uint16 const* e = q + bu.getTotalPulses();

	if(q==e) { xlogline("num_pulses=0"); return; }

	if(last_pulse_is_pause)
	{
		blen -= 1; num_pulses -= 1;
		uint32 pause_cc = * --e;

		while(e>q && *(e-1)==0)
		{
			e -= 2;
			assert(*e==0xffff);
			pause_cc += 0xffff;
			blen-=2; num_pulses-=2;
		}

		pause_ms = uint16(minmax(1e-3, pause_cc*SPCC, 10.0) * 1000 + 0.5);

		if(q==e) { xlogline("num_pulses=0 after replacing last pulse with pause"); return; }
	}

	// count num_pulses and calculate blen:
	while(q<e)
	{
		uint32 cc = *q++;
		if(cc < 0xff8) continue;		// +1 byte, +1 pulse
		blen += 4;						// will be a long pulse
		while(q<e && *q==0) { q+=2; blen-=2; num_pulses-=2; } // join pulses from bu[]
	}

	// allocate buffer:
	data = new uint8[blen-10];
	uint8* z = data;
	q = bu.getData();	// rewind

	// store pulses in CSW file format:
	while(q<e)
	{
		uint32 cc = *q++;
		if(cc < 0xff8) { *z++ = (cc+8)>>4; continue; }	// store short pulse

		// store extended pulse:
		while(q<e && *q==0)
		{
			q++;
			cc += *q++;
		}

		*z++ = 0;
		poke4Z(z,(cc+8)>>4);
		z += 4;
	}

	assert(z-data == blen-10);
}



/*	convert O80Data to TzxBlock
	• the converted signal is prepended with 5 sec low pulse
	  caller may subtract this from the pause of a preceding block
	• a pause of 1 sec is appended at the end of the block
	  TODO: this needs to be updated if a pause duration is added to O80Data
	• "next level" after this block is 0 (as after TzxPause)

	Note: Die 5s Pause waren notwendig, weil die Laderoutine klaglos beliebige Pulsfolgen
	in Bits umwandelt. Man musste erst das LOAD-Kommando auf dem ZX80 vorbereiten,
	dann das Band starten, dann warten, dass die Wiedergabe still war und dann das
	LOAD-Kommando abschicken. Auf keinen Fall durfte eine Flanke vor dem ersten Bit
	erkannt werden! Alle Daten würden verschoben geladen und die ganze Warterei war umsonst…
*/
TzxBlock19::TzxBlock19( O80Data const& q )
:
	TzxBlock(0x19,yes),
	p_symdef(NULL),
	p_data(NULL),
	d_symdef(NULL),
	d_data(NULL)
{
	// pilot: one low pulse for 1/4 sec:
	p_symdef_cnt=1;				// Number of symbols in the pilot alphabet table (0=256)
	p_symdef=new SymDef[1];		// if p_data_cnt>0: symdef[p_symdef_cnt]
	p_symdef_max=1;				// Maximum number of pulses per pilot symbol

	p_symdef[0].flags = 0b10;	// force low level
	p_symdef[0].pulses = 1;
	p_symdef[0].data = new uint16[1+1];
	p_symdef[0].data[0] = 35000;// 1/100 sec
	p_symdef[0].data[1] = 0;	// stopper

	p_data_cnt=1;				// Total number of symbols in pilot data block (can be 0)
	p_data=new uint8[1*3];		// if p_data_cnt>0: PRLE[p_data_cnt] pilot data stream

	p_data[0] = 0;				// symbol index
	poke2Z(p_data+1,500);		// repeat: 500 * 1/100 sec = 5 sec

	// data:
	d_data_cnt   = q.count()*8;	// Total number of symbols in data stream (can be 0)
	d_symdef_max = 18;			// Maximum number of pulses per data symbol
	d_symdef_cnt = 2;			// Number of symbols in the data alphabet table (0=256)

	bits_per_symbol = 1;			// ceil(log2(d_symdef_cnt));
	mask_databits   = 1;			// (1<<bits_per_symbol)-1;
	num_databits    = q.count()*8;	// bits_per_symbol*d_data_cnt;
	num_databytes   = q.count();		// (num_databits+7)/8;

	const uint hipuls = 530  *350/325;
	const uint lopuls = 520  *350/325;
	const uint bitgap = 4689 *350/325;

	uint16 qdata[19] = { hipuls,lopuls, hipuls,lopuls, hipuls,lopuls, hipuls,lopuls, hipuls,lopuls,
						 hipuls,lopuls, hipuls,lopuls, hipuls,lopuls, hipuls,bitgap, 0 };

	d_symdef = new SymDef[2];		// if d_data_cnt>0: symdef[d_symdef_cnt]

	d_symdef[0].flags = 3;			// force high
	d_symdef[0].pulses = 8;
	d_symdef[0].data = new uint16[19];
	memcpy(d_symdef[0].data, qdata+10, 9*sizeof(uint16));

	d_symdef[1].flags = 3;			// force high
	d_symdef[1].pulses = 18;
	d_symdef[1].data = new uint16[19];
	memcpy(d_symdef[1].data, qdata, 19*sizeof(uint16));

	d_data = new uint8[q.count()];	// if d_data_cnt>0: data[num_databytes] pilot data stream
	memcpy(d_data,q.getData(),q.count());

	pause_ms = 1000;					// pause after this block [ms]

	blen = 2 // pause_ms
			+ 4	// p_data_cnt
			+ 1	// p_symdef_max
			+ 1	// p_symdef_cnt
			+ 4	// d_data_cnt
			+ 1	// d_symdef_max
			+ 1;// d_symdef_cnt

	blen += p_symdef_cnt * (1 + 2 * p_symdef_max);	// p_symdef[]
	blen += p_data_cnt * 3;							// p_data[]
	blen += d_symdef_cnt * (1 + 2 * d_symdef_max);	// d_symdef[]
	blen += d_data_cnt/8;							// d_data[]
}



// ##################################################################################
//      c'tor et.al.
// ##################################################################################


TzxData::~TzxData()
{
	delete data;
}


TzxData::TzxData(TzxBlock* data, TrustLevel trustlevel)
:
	TapeData(isa_TzxData,trustlevel),
	data(data)
{}


/*  write TZX file:
	create new tzx file from tapeblocks
	may overwrite existing file
*/
//static
void TzxData::writeFile( cstr fpath, TapeFile& tapeblocks, TzxConversionStyle style ) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("TzxData::writeFile(%s)",fpath);
	FD fd(fpath,'w');							// throws; may overwrite

	fd.write_bytes("ZXTape!\x1A\x01\x20",10);	// version 1.20 header

	for(uint i=0;i<tapeblocks.count();i++)
	{
		TapeFileDataBlock* block = tapeblocks[i];
		if(block->isEmpty()) continue;

/*	if style==default
		if tzxdata -> write(tzxdata)
		if getTapData().conversion_success -> write tzxdata(tapdata)
		if getO80Data().conversion_success -> write tzxdata(o80data)
		else store tzxdata(csw,default) --> Block18 'csw data'

	if style==idealize
		if getTapData().conversion_success -> write tzxdata(tapdata)
		if getO80Data().conversion_success -> write tzxdata(o80data)
	//	if tzxdata.original_data -> write tzxdata.idealize()			future: only rework csw or sample block
		else write tzxdata(csw,idealize) --> Block12,15,o.Ä.

	if style==exact
		if tzxdata.original_data -> write tzxdata						may be a conversion from tap or o80!
		if getTapData().original_data -> write tzxdata(tapdata)
		if getO80Data().original_data -> write tzxdata(o80data)
		else write tzxdata(csw,exact) --> Block18 'csw'
*/

		TzxData* tzxdata = block->tzxdata;

		switch(style)
		{
			TapData* tapdata;
			O80Data* o80data;

		case TzxConversionDefault:
			if(tzxdata) break;

		case TzxConversionIdealize:
			tzxdata =
				(tapdata = block->getTapData()) && tapdata->trust_level>=conversion_success ? new TzxData(*tapdata) :
				(o80data = block->getO80Data()) && o80data->trust_level>=conversion_success ? new TzxData(*o80data) :
			//	style==idealize && tzxdata && tzxdata.trustlevel==original_data ? tzxdata.idealize() :
				new TzxData(*block->cswdata->normalize(),style);
			break;

		case TzxConversionExact:
			tzxdata =
				tzxdata && tzxdata->trust_level==original_data ? tzxdata : // may be already a conversion from tap|o80
				(tapdata = block->getTapData()) && tapdata->trust_level==original_data ? new TzxData(*tapdata) :
				(o80data = block->getO80Data()) && o80data->trust_level==original_data ? new TzxData(*o80data) :
				new TzxData(*block->cswdata->normalize(),style);
			break;
		}

		if(block->tzxdata != tzxdata) delete block->tzxdata;
		block->tzxdata = tzxdata;
		tzxdata->data->writeToFile(fd);
	}
}


/*  read TZX file
	and split into blocks

	An emulator should put the 'current pulse level' to 'low' when starting to play a TZX file,
	either from the start or from a certain position.
*/
//static
void TzxData::readFile(cstr fpath, TapeFile& tapeblocks) noexcept(false) // file_error,data_error,bad_alloc
{
	xlogIn("TzxData::readFile(%s)",fpath);

	TempMemPool tmp;
	try
	{
		FD fd(fpath,'r');					// throws
		if(fd.file_size()==0) return;		// empty file => empty tape

	// header:
		char magic[8]; fd.read_bytes(magic,8);
		if(memcmp(magic,"ZXTape!\x1A",8)) throw data_error("not a tzx file");
		uint version_h = fd.read_uint8();
		uint version_l = fd.read_uint8();
		if(version_h!=1) throw data_error("tzx: version %i.%02i not supported",version_h,version_l);

		//											has		is a	sets
		//											data	stopper	phase0
		// 0x10	Standard speed data block			y		?pause	n
		// 0x11	Turbo speed data block				y		?pause	n
		// 0x12	Pure tone							y		n		n
		// 0x13	Sequence of pulses of var. length	y		n		n
		// 0x14	Pure data block						y		?pause	n
		// 0x15	Direct recording block				y		?pause	y
		// 0x18	CSW recording block					y		?pause	n
		// 0x19 Generalized data block				y		?pause	y|n
		// 0x20 Pause or 'Stop the tape' command	y		y		n
		// 0x21 Group start							y		y		y|n
		// 0x24 Loop start							y		n		y|n
		// 0x2A Stop the tape if in 48K mode		y		y		n
		// 0x2B Set signal level					y		n		y

		// 0x30 Text description					n		n		n
		// 0x31 Message block						n		n		n
		// 0x32 Archive info						n		n		n		denk: sep. speichern?
		// 0x33 Hardware type						n		n		n		denk: sep. speichern?

		for(;;)
		{
			TzxBlock* block = TzxBlock::readFromFile(fd);
			if(!block) return;
			tapeblocks.append(new TzxData(block,original_data));
		}
	}
	catch(any_error& e)
	{
		e.text = xdupstr(e.text);
		throw e;						// hiernach ist's ein any_error ...  :-(
	}
}


/*	get block info, if any:
*/
cstr TzxData::getMajorBlockInfo() const noexcept
{
	for(TzxBlock const* p = data; p; p=p->next)
	{
		cstr info = p->get_info();
		if(info) return info;
	}
	return NULL;
}


/*	get minor block info, if any:
*/
cstr TzxData::getMinorBlockInfo() const noexcept
{
	for(TzxBlock const* p = data; p; p=p->next)
	{
		cstr info = p->get_info2();
		if(info) return info;
	}
	return NULL;
}


/*	CONVERT TapData to TzxData:
	• "trust level" is set to original trust level, even if 'original_data' (=> as good as the original data)

	• first block is a polarity setter which works as in new CswBuffer(TapData)
	  if number of pilot pulses is even, then first puls is high: as it was saved
	  if number of pilot pulses is odd then first pulse is low:   assuming the real first puls was hidden
	  assuming an initial phase 0 this should be a nop for ZX Spectrum and a toggle for Jupiter Ace.
	• then comes the main block, which is a Block10 (ZX Spectrum) or Block11 (Jupiter Ace)
	• Jupiter Ace:
	  the Ace adds an additional final pulse (903@3.25MHz),
	  therefore the pause of the main block is set to 0,
	  and an additional block with just this pulse and a final pause block are appended.
	• "next phase" after this block is 0 (as after TzxPause).
	  there is at least 1ms pause at end of this block.
*/
TzxData::TzxData(TapData const& q)
:
	TapeData(isa_TzxData,q.trust_level),
	data(NULL)
{
	data = new TzxBlock2B((q.pilot_pulses&1)^1);	// set polarity: even|odd => start with hi|lo pulse

	if(q.isJupiter())
	{
		TzxBlock11* dat = new TzxBlock11(q);	// allows using the slightly different timing
		dat->pause_ms = 0;						// no pause at end of this block
		data->append(dat);
		data->append(new TzxBlock12(903*350/325,1));	// final pulse
		data->append(new TzxBlock20(uint16(minmax(1000.0, q.pause*1000 + (0.5-903/3250.0), 10000.0))));	// pause
	}
	else	// ZX Spectrum or unknown:
	{
		data->append(new TzxBlock10(q));	// standard tape block
	}
}


/*	CONVERT O80Data to TzxData:
	• "trust level" is set to original trust level, even if 'original_data' (=> as good as the original data)
	• the TzxBlock is always saved with not-inverted polarity
	• the converted signal is prepended with 5 sec low pulse
	  caller may subtract this from the pause of a preceding block
	• there is 1sec pause at end of this block
	  TODO: this needs to be updated if a pause duration is added to O80Data
	• "next level" after this block is 0 (as after TzxPause)

	Die Pause wird als normale Pause im TzxBlock gespeichert.
	=> Die Pause wird im Vgl. zum letzten Puls getoggelt und ist damit positiv.
	=> Vgl.: O80Data hängt die Pause nicht-invertiert an, verlängert also die letzte Bit-Gap. Beides geht wohl.
*/
TzxData::TzxData(O80Data const& q)
:
	TapeData(isa_TzxData,q.trust_level),
	data(NULL)
{
	data = new TzxBlock19(q);
}


/*	CONVERT CswBuffer to TzxData:
	• should not be called if the data can be decoded as TapData or O80Data
	• CswBuffer must be normalized
	• CswBuffer must not be empty

	• TzxConversionStyle is currently ignored  TODO

	• returned block is prepended with a Block2B 'set polarity', so it is always read back non-inverted
	• the last pulse is replaced with pause which is limited to 1ms to 10s
	• 'next phase' after this block is 0 as after TzxPause

	• if num_pulses ≥ 2 then a Block18 'csw data' is returned
	• if num_pulses = 1 then a Block20 'pause' is returned instead
*/
TzxData::TzxData(CswBuffer const& bu, TzxConversionStyle/*currently ignored TODO*/)
:
	TapeData(isa_TzxData),
	data(NULL)
{
	assert(bu.getTotalCc()>0);
	assert(bu.is_normalized());

	data = new TzxBlock2B(bu.getPhase0());		// set polarity

	TzxBlock18* dat = new TzxBlock18(bu,yes);
	if(dat->num_pulses)
		data->append(dat);
	else
	{
		TzxBlock20* gap = new TzxBlock20(dat->pause_ms);
		delete dat;
		data->append(gap);
	}
}


/*  CONVERT TzxData to CswBuffer:
	note: this CswBuffer method is declared as a friend

	TZX file spec:
	• Die initial Phase des ersten Blocks auf einem Band ist "low".
	• Die initial Phase des nächsten Blocks nach einer Pause ist "low".
	=> Ein TzxData-Block startet immer mit einer Initial Phase von "low".
	• Die Cpu-Frequenz für alle CC-Angaben im Tzx-File ist immer 3,5MHz.
*/
CswBuffer::CswBuffer(TzxData const& tzxdata, uint32 ccps)
:
	CswBuffer(ccps,0,666)
{
	xlogIn("new CswBuffer(TzxData&)");

	// resize to estimated size:
	// 48k = 8000+48000*16 = 776000
	grow(880000);

	// store pulses:
	tzxdata.data->storeCsw(*this);

	assert(getPhase0()==0);

//	if(ccps!=3500000)			// resampling required?
//	{
//		CswBuffer* z = new CswBuffer(*this,ccps);
//		delete[] data; data=z->data; z->data=NULL;
//		max = z->max;
//		end = z->end;
//		cc_end = z->cc_end;
//		phase = z->phase;		// should not have changed
//		this->ccps = ccps;
//		delete z;
//	}
//	else
	shrinkToFit();
}


/*	CONVERT TzxData to TapData
	a TzxBlock may contain an easily convertible TzxBlock10 or TzxBlock11
	else render to CswBuffer and try to decode
	evtl. we can tell beforehand that it's not convertible
*/
TapData::TapData( TzxData const& /*q*/ )
:
	TapData(no,no)
{
	TODO();
}


/*	CONVERT TzxData to TapData
	a TzxBlock may contain an easily convertible TzxBlock10 or TzxBlock11
	else render to CswBuffer and try to decode
	evtl. we can tell beforehand that it's not convertible
*/
O80Data::O80Data( TzxData const& /*q*/ )
:
	O80Data()
{
	TODO();
}


























