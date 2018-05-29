/*	Copyright  (c)	Günter Woigk 2014 - 2018
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


#ifndef SP0256_H
#define SP0256_H

#include "kio/kio.h"
#include "DspTime.h"


class SP0256
{
	const float32	rc;		// RC value (for both stages), e.g. RC = 33kΩ * 22nF
	const float32	ff;		// filter factor per audio-out sample (for both filter stages)
	Time	time_per_sample;// SP0256: time[seconds] per sample = 1.0 / (XTAL/312)
	Sample	volume;
    Sample	amplification;	// volume setting (incl. all other factors)
	bool	hifi;

	// coroutine state machine
	uint	sm_state;		// for coroutine macros
	Time	time;			// up to which time the state machine ran:
	Time	time_sample_end;// time when the currently rendered 10kHz sample ends
	Sample	sample;
	Sample	sample_at_c1;	// current filtered output value at C1 (output of filter stage 1)
	Sample	sample_at_c2;	// current filtered output value at C2 (output of filter stage 2)

	void	run_statemachine(Time);
	void	output_filtered(Sample,Time);

	// 17 sound and filter registers:
	uint	current_opcode;
	uint 	repeat;			// 6 bit: ≥ 1
	uint8 	pitch;			// 8 bit: 0 -> white noise
	uint8 	amplitude;		// 8 bit: bit[7…5] = exponent, bit[4…0] = mantissa
	uint8	c[12];			// 8 bit: filter coefficients b and f
	int8 	pitch_incr;		// Δ update value applied to pitch after each period
	int8 	amplitude_incr;	// Δ update value applied to amplitude after each period

	int 	_c[12];			// int10:  filter coefficients b and f (bereits umgerechnet)
	int 	_z[12];			// feedback values
	uint16	_shiftreg;		// noise
	uint	_i;

	// microsequencer registers:
	uint 	mode;			// 2 bit	from SETMODE
	//uint 	repeat_prefix;	// 2 bit	from SETMODE	(already shifted left 4 bits)
	uint 	page;			// 4 bit	(already bit-swapped and shifted left 12 bits)
	uint 	pc;				// 16 bit
	uint 	stack;			// 16 bit single level return "stack"
	uint 	command;		// 8 bit current/next command
	uint	current_command;// 8 bit current command

	bool	stand_by;		// 1 = stand by (utterance completed)
	bool	command_valid;	// 1 = command valid == !LRQ (load request)

	// speech rom AL2:		((presumably "american language" vs.2))
	uint8	rom[2048];
	uint	byte;			// current/last byte read from rom, remaining valid bits are right-aligned
	uint	bits;			// number of valid bits remaining

public:
	// Statistics
	struct Stats
	{
		Sample	max_sample;
		Sample	max_sample_at_c1;
		Sample	max_sample_at_c2;
		Stats():max_sample(0),max_sample_at_c1(0),max_sample_at_c2(0){}
	};
	Stats	allophone_stats[64];
	Stats	opcode_stats[16];

public:
	SP0256(cstr romfilepath, bool bitswapped, float RC = 33e3f * 22e-9f);
	~SP0256();

	void	powerOn(/*Time t=0,*/ Sample volume = 1.0f, Frequency xtal = 3.12e6);
	void	setHifi(bool f)				{ hifi = f; }
	bool	isHifi()					{ return hifi; }
	void	setVolume(Time,Sample);
	void	setClock(Time,Frequency);
	void	writeCommand(Time,uint);
	bool	isStandby(Time);			// test stand_by
	bool	isBusy(Time);				// test command_valid
	bool	audioBufferEnd(Time);
	void	reset(Time, Sample volume, Frequency xtal);

private:
	void	disassAllophones();

	void	cmdSetPage(uint opcode);	// or RTS
	void	cmdLoadAll();
	void	cmdLoad2C(uint instr);
	void	cmdLoad2();
	void	cmdSetMsb35A(uint instr);
	void	cmdSetMsb3();
	void	cmdLoad4();
	void	cmdSetMsb5();
	void	cmdSetMsb6();
	void	cmdJmp(uint opcode);
	void	cmdSetMode(uint opcode);
	void	cmdDelta9();
	void	cmdSetMsbA();
	void	cmdJsr(uint opcode);
	void	cmdLoadC();
	void	cmdDeltaD();
	void	cmdLoadE();
	void	cmdPause();

	void	logSetPage(uint opcode);	// or RTS
	void	logLoadAll();
	void	logLoad2C(uint instr);
	void	logLoad2();
	void	logSetMsb35A(uint instr);
	void	logSetMsb3();
	void	logLoad4();
	void	logSetMsb5();
	void	logSetMsb6();
	void	logJmp(uint opcode);
	void	logSetMode(uint opcode);
	void	logDelta9();
	void	logSetMsbA();
	void	logJsr(uint opcode);
	void	logLoadC();
	void	logDeltaD();
	void	logLoadE();
	void	logPause();

	uint8	next8();
	uint8	nextL(uint n);
	uint8	nextR(uint n);
	int8	nextSL(uint n);
	int8	nextSR(uint n);
	void	set_clock(Frequency);
	void	set_volume(Sample);
};


#endif











