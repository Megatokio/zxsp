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



#ifndef AUDIODATA_H
#define AUDIODATA_H

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


#endif



















