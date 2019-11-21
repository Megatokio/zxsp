#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

#include "DspTime.h"
#include "StereoSample.h"

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



















