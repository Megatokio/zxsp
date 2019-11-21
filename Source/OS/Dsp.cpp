/*	Copyright  (c)	Günter Woigk 2019 - 2019
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

#include "kio/kio.h"
#include "Dsp.h"
#include "Uni/globals.h"
#include <math.h>

Time		system_time			= 0.0;
Frequency	samples_per_second	= 44100;

namespace Dsp
{
StereoSample audio_out_buffer [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];
StereoSample audio_in_buffer  [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];

bool audio_input_device_present	= no;	// preset to no if error
bool audio_input_device_enabled	= no;	// preset to no if error
bool audio_output_device_enabled = yes;	// must be present
Sample audio_output_volume = 0.3f;

void enableAudioInputDevice (bool f)
{
	if (f && !audio_input_device_present)
	{
		showWarning( "No audio input device found." );
	}
	audio_input_device_enabled = f && audio_input_device_present;
}

void enableAudioOutputDevice (bool f)
{
	audio_output_device_enabled = f;
}

void setOutputVolume (Sample volume)
{
	if (volume <= 0.0f)
	{
		audio_output_device_enabled = off;
	}
	else
	{
		audio_output_volume = fminf(volume,1.0f);
		audio_output_device_enabled = on;
	}
}

void outputSamples (cStereoSample& sample, Time aa/*start [seconds]*/, Time ee/*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second;		// sample-based
	ee *= samples_per_second;		// ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a>=0);
	assert(e>=a);
	assert(e<DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa-floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee-floor(ee));

	while (pa<pe) { *pa++ += sample; }
}

void outputSamples (Sample sample, Time aa/*start [seconds]*/, Time ee/*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second;		// sample-based
	ee *= samples_per_second;		// ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a>=0);
	assert(e>=a);
	assert(e<DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa-floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee-floor(ee));

	while (pa<pe) { *pa++ += sample; }
}

}	// namespace Dsp































