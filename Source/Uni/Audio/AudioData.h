#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeData.h"
#include "TapeFile.h"
#include "audio/AudioDecoder.h"
// #include "Templates/Ptr.h"
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
	friend CswBuffer::CswBuffer(const AudioData&, uint32);

	Array<Sample> float_samples;
	Array<int16>  int16_samples;
	bool		  stereo;			  // in buffers, not neccessarily in file
	uint32		  samples_per_second; // should be sample rate of original data

	std::shared_ptr<AudioDecoder> audio_decoder; // if any, then audio_data may be empty
	uint32						  adc_start_pos; // in soundfile, samples (soundfile sample rate)
	uint32						  adc_end_pos;	 // in soundfile, samples (soundfile sample rate)
	uint32						  adc_num_samples() const { return adc_end_pos - adc_start_pos; }

public:
	AudioData();
	explicit AudioData(const AudioData&);
	explicit AudioData(const TapeData&);
	explicit AudioData(const CswBuffer&, uint32 sps);
	explicit AudioData(std::shared_ptr<AudioDecoder>, uint32 a, uint32 e);
	virtual ~AudioData() override;

	static void readFile(cstr fpath, TapeFile&);
	static void writeFile(cstr fpath, TapeFile&);
};
