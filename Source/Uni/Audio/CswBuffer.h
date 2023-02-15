#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	Buffer for csw bytes: runlength encoded compressed square wave data.
	Stores an audio signal as one-bit samples, and compacts this further
	by storing the run length (sample count) of each phase only.

	Definitions:
		pulse:	sequence of samples with all the same phase
		phase:	boolean value: sample<0 ? 0 : 1

	The buffer may be in one of 7 modes:
		- stopped: tape operations like cueing are possible
		playing:
		- samples: play into a sample buffer for audio output
		- edges:   peek into the data with input(Time) instructions
		- pulses:  retrieve complete pulses for data conversion (export)
		recording:
		- samples: record from a sample buffer from audio input
		- edges:   store signal edges from output(Time) instructions
		- pulses:  store complete pulses for data conversion (import)

	Compression scheme:
		1 byte:	 1 to 255 samples
		5 bytes: 0 or more than 255 samples:
				 char(0)
				 char(m)
				 char(m>>8)
				 char(m>>16 + e<<4)		--> count = (m+$100000)<<e
				 char(0)
		- can store pulses of any length in one chunk of data: never need to split and test for splitted pulses
		- backward skipping is possible (for tape cueing and pulse merging)
		- signal phase can be derived from the phase of the first pulse and pos&1, but does it exactly vice versa.
		- pulses can be stored without actually knowing the current phase.
		! pulses of length 0 must be stored as 5 bytes.

	pos		is the index in data[] of the current pulse
	cc_pos	is the corresponding amount of samples up to pos
	now		is the corresponding system time (edges only)
	end		is the index in data[] of the used data end == size of used data
	cc_end	is the corresponding amount of samples up to end
	phase	is the current phase.
			- playing:	 this is the phase of the currently played pulse, which is the pulse before pos
			- recording: this is the phase of the currently recorded pulse, which is the pulse at pos
	akku:	is the Time offset to pos determining the exact tape position inside a pulse:
			tape position = TimeForPulses(cc_pos) + akku.
			- playing:	 this is the time remaining for the prev pulse to read pos: akku<0.
			- recording: this is the time akkumulated so for for the currently recorded pulse: akku>0.
			due to rounding akku may refer ±0.5 into the opposite pulse.
*/

#include "zxsp_types.h"

/*  Notes:

	total samples = 2^32-1  ==  20 minutes at 3.5MHz
	pre-growing with 1100000 pulses is almost always sufficient (zxsp, 64k)
	pre-growing with 2600000 pulses is almost always sufficient (zx81, 16k)
	or pre-grow with 10000+16*size_of_tap_file (zxsp)
	purge buffer before writing
	rewind or seek position before reading
	pulses longer than 0xFFFF samples are split and interleaved with 0-len opposite pulses


	Read and write with CPU input / output:
	---------------------------------------

	Write to tape with CPU output:

		1.  create new CswBuffer or purge existing buffer
		2.  pre-grow buffer (optional)
		3.  startRecording(CC)
		4a. output(CC,bit) for every output instruction
		4b. seek(CC) for audioBufferEnd() or videoFrameEnd() to update current CC
		5.  stopRecording(CC)

	Read from tape with CPU input:

		1.  create and fill CswBuffer from TapeData
		2.  rewind() or seek(CC) tape
		3a. input(CC) for every input instruction
		3b. seek(CC) for audioBufferEnd() or videoFrameEnd() to update current CC


	Read and write pulses
	---------------------

	this is for converting to or from TapeData.
*/


typedef uint32 CC;


class CswBuffer
{
	friend class TapeFileDataBlock;
	friend class TapeRecorder;

	uint16* data;	// buffer
	uint32	max;	// allocated size
	uint32	end;	// used size
	CC		cc_end; // total samples up to 'end'
	uint32	ccps;	// ccps = (nominelle) CPU-Clock der laufenden Maschine

	bool recording;

	mutable uint32 pos;		  // current index in 'data'
	mutable CC	   cc_pos;	  // total samples up to 'pos' (excl.)
	mutable bool   phase;	  // current phase
	mutable CC	   cc_offset; // current CC offset inside current sample

	void skip() const noexcept;
	void rskip() const noexcept;
	void grow(uint32) throws /*bad alloc*/;

public:
	~CswBuffer() noexcept { delete[] data; }
	CswBuffer(uint32 ccps, bool phase0, int foo);
	CswBuffer(const TapData&, uint32 ccps);	  // implemented in TapData.cpp
	CswBuffer(const TzxData&, uint32 ccps);	  // implemented in TzxData.cpp
	CswBuffer(const O80Data&, uint32 ccps);	  // implemented in O80Data.cpp
	CswBuffer(const RlesData&, uint32 ccps);  // implemented in RlesData.cpp
	CswBuffer(const AudioData&, uint32 ccps); // implemented in AudioData.cpp
	CswBuffer(const TapeData&, uint32 ccps);
	CswBuffer(const CswBuffer&, uint32 ccps);
	CswBuffer(const int16* samples, uint32 count, uint32 sps, uint32 ccps);
	CswBuffer(const CswBuffer&)			   = delete;
	CswBuffer& operator=(const CswBuffer&) = delete;

	void growBuffer(uint32) throws /*bad alloc*/;
	void purge() noexcept;
	void shrinkToFit() noexcept;

	uint32 ccPerSecond() const noexcept { return ccps; }

	bool isAtStart() const noexcept { return cc_pos + cc_offset == 0; }
	bool isAtEnd() const noexcept { return pos >= end; }
	void seekStart() const noexcept;
	void seekEnd() const noexcept;
	CC	 seekCc(CC) const noexcept;
	void seekPos(uint32) const noexcept;
	Time seekTime(Time t) const noexcept
	{
		seekCc(t * ccps);
		return t;
	}

	CC		   getTotalCc() const noexcept { return cc_end; }
	Time	   getTotalTime() const noexcept { return Time(cc_end) / ccps; }
	uint32	   getTotalPulses() const noexcept { return end; }
	uint32	   getCurrentPos() const noexcept { return pos; }
	CC		   getCurrentCc() const noexcept { return cc_pos + cc_offset; }
	Time	   getCurrentTime() const noexcept { return Time(cc_pos + cc_offset) / ccps; }
	bool	   getCurrentPhase() const noexcept { return phase; }
	bool	   getPhase0() const noexcept { return (pos & 1) ^ phase; }
	bool	   getFinalPhase() const noexcept { return ((end - pos) & 1) ^ phase; }
	uint16*&   getData() noexcept { return data; }
	cu16ptr	   getData() const noexcept { return data; }
	void	   invertPolarity() noexcept { phase ^= 1; } // polarity of entire buffer!
	CswBuffer* normalize() noexcept;
	bool	   is_normalized() const noexcept;
	bool	   isSilenceOrNoise() const noexcept;

	void splitAtCurrentPos(CswBuffer&);
	void addToAudioBuffer(
		StereoSample*,
		uint	count,
		double	samples_per_second,
		double& zpos,
		uint32& qpos,
		double& qoffs,
		Sample	volume = 0.32f); // 0.32 ~ -10dB

	// read / write with CPU input / output:

	bool isRecording() const noexcept { return recording; }
	bool isPlaying() const noexcept { return !recording; }
	bool inputCc(CC) const noexcept;
	void outputCc(CC, bool) throws /*bad alloc*/;
	void stopRecording(CC) throws /*bad alloc*/;
	void startRecording(CC) noexcept;
	bool inputTime(Time) const noexcept;
	void outputTime(Time, bool) throws /*bad alloc*/;


	// read / write pulses:
	// for TapeData conversion

	CC	 readPulse() const noexcept;
	void putbackPulse() const noexcept;
	void skipSilence(uint N = 7) const noexcept;
	void setPhase(bool) throws /*bad alloc*/;
	void writePulseCc(CC) throws /*bad alloc*/;
	void appendToPulseCc(CC) throws /*bad alloc*/;
	void writePulseCc(CC, bool) throws /*bad alloc*/;
	void writePulse(Time s) throws /*bad alloc*/ { writePulseCc(CC(s * ccps + 0.5)); }
	void writePulse(Time s, bool p) throws /*bad alloc*/ { writePulseCc(CC(s * ccps + 0.5), p); }
	void appendToPulse(Time s) throws /*bad alloc*/ { appendToPulseCc(CC(s * ccps + 0.5)); }

	void writePureTone(uint32 num_pulses, Time seconds_per_pulse);
	void writePureData(cu8ptr bu, uint32 total_bits, Time bit0, Time bit1);
	void writeTzxPause(Time);
};
