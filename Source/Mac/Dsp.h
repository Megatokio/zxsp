/*	Copyright  (c)	Günter Woigk 2002 - 2018
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

#ifndef DSP_H
#define DSP_H

#include "Audio/DspTime.h"
#include "Audio/StereoSample.h"

typedef float	Sample;			typedef Sample const		cSample;
        class	StereoSample;	typedef StereoSample const	cStereoSample;


namespace Dsp
{
extern	StereoSample	audio_out_buffer[];
extern	StereoSample	audio_in_buffer[];

extern	bool	audio_input_device_present;
extern	bool	audio_input_device_enabled;
extern	bool	audio_output_device_enabled;
extern	Sample	audio_output_volume;


extern	void	startCoreAudio				(bool input_enabled);//, int playthrough_mode );
extern	void	stopCoreAudio				( );

inline	bool	isAudioInputDevicePresent	( )			{ return audio_input_device_present; }
inline	bool	isAudioInputDeviceEnabled	( )			{ return audio_input_device_enabled; }
extern	void	enableAudioInputDevice		( bool f );

inline	bool	isAudioOutputDevicePresent	( )			{ return 1/*yes*/; }
inline	bool	isAudioOutputDeviceEnabled	( )			{ return audio_output_device_enabled; }
extern	void	enableAudioOutputDevice		( bool f );

extern	void	setOutputVolume				( Sample volume );
inline	Sample	getOutputVolume				( )			{ return audio_output_volume; }

//extern	void	setPlaythrough				( uint mode );
//		enum
//		{		playthrough_minus10dB,
//				playthrough_minus30dB,
//			 	playthrough_off
//		};

extern	void	outputSamples				( cStereoSample&, Time start, Time end );
extern	void	outputSamples				( Sample, Time start, Time end );

// ---- Utilities ----

inline void shiftBuffer(StereoSample*bu)
			{ for( int i=0; i<DSP_SAMPLES_STITCHING; i++ ) bu[i] = bu[DSP_SAMPLES_PER_BUFFER+i]; }
inline void clearBuffer(StereoSample*bu)	// preserves stitching at buffer start
			{ for( int i=DSP_SAMPLES_STITCHING; i<DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING; i++ ) bu[i] = 0.0; }


}

#endif


















