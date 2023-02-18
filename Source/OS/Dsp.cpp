// Copyright (c) 2019 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Dsp.h"
#include "Uni/globals.h"
#include "kio/kio.h"
#include <math.h>

Time	  system_time		 = 0.0;
Frequency samples_per_second = 44100;

namespace os
{
StereoSample audio_out_buffer[DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING];
StereoSample audio_in_buffer[DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING];

bool   audio_input_device_present  = no;  // preset to no if error
bool   audio_input_device_enabled  = no;  // preset to no if error
bool   audio_output_device_enabled = yes; // must be present
Sample audio_output_volume		   = 0.3f;

void enableAudioInputDevice(bool f)
{
	if (f && !audio_input_device_present) { showWarning("No audio input device found."); }
	audio_input_device_enabled = f && audio_input_device_present;
}

void enableAudioOutputDevice(bool f) { audio_output_device_enabled = f; }

void setOutputVolume(Sample volume)
{
	if (volume <= 0.0f) { audio_output_device_enabled = off; }
	else
	{
		audio_output_volume			= fminf(volume, 1.0f);
		audio_output_device_enabled = on;
	}
}

void outputSamples(const StereoSample& sample, Time aa /*start [seconds]*/, Time ee /*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second; // sample-based
	ee *= samples_per_second; // ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a >= 0);
	assert(e >= a);
	assert(e < DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa - floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee - floor(ee));

	while (pa < pe) { *pa++ += sample; }
}

void outputSamples(Sample sample, Time aa /*start [seconds]*/, Time ee /*end [seconds]*/)
{
	// Output sample value to audio_out_buffer[]:

	aa *= samples_per_second; // sample-based
	ee *= samples_per_second; // ""

	int32 a = int32(aa);
	int32 e = int32(ee);

	assert(a >= 0);
	assert(e >= a);
	assert(e < DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING);

	StereoSample* pa = audio_out_buffer + a;
	*pa -= sample * Sample(aa - floor(aa));

	StereoSample* pe = audio_out_buffer + e;
	*pe += sample * Sample(ee - floor(ee));

	while (pa < pe) { *pa++ += sample; }
}

} // namespace Dsp
