#pragma once
/*	Copyright  (c)	Günter Woigk 2000 - 2019
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

#include "TapeData.h"
#include "TapeFile.h"
#include "audio/AudioDecoder.h"
//#include "Templates/Ptr.h"
#include "Templates/RCPtr.h"


/*	class AudioData
	---------------

	Subclass of "TapeData".

	Mono or stereo audio data block

	A "AudioData" instance contains data from an audio file or
	from CswBuffer or other TapeData classes converted to audio samples.
	A "AudioData" instance contains one data block.

	As all TapeData classes AudioData defines or implements conversion creators for CswBuffer:
	- new AudioData(CswBuffer const&) and
	- new CswBuffer(AudioData const&, uint32 ccps).

	And static file i/o methods:
	readFile (cstr fpath, TapeFile&) and writeFile (cstr fpath, TapeFile&)

note:
	Framework	AudioToolbox/AudioToolbox.h
	Declared in	AudioToolbox/AudioFile.h
*/


class AudioData : public TapeData
{
	friend CswBuffer::CswBuffer(AudioData const&, uint32);

	Array<Sample>	float_samples;
	Array<int16>	int16_samples;
	bool			stereo;					// in buffers, not neccessarily in file
	uint32			samples_per_second;		// should be sample rate of original data

	RCPtr<AudioDecoder> audio_decoder;	// if any, then audio_data may be empty
	uint32			adc_start_pos;			// in soundfile, samples (soundfile sample rate)
	uint32			adc_end_pos;			// in soundfile, samples (soundfile sample rate)
	uint32			adc_num_samples() const { return adc_end_pos - adc_start_pos; }

public:
	AudioData();
	explicit AudioData(const AudioData&);
	explicit AudioData(const TapeData&);
	explicit AudioData(const CswBuffer&, uint32 sps);
	explicit AudioData(AudioDecoder*, uint32 a, uint32 e);
	virtual ~AudioData();

	static void	readFile(cstr fpath, TapeFile&) throws;
	static void	writeFile(cstr fpath, TapeFile&) throws;
};






















