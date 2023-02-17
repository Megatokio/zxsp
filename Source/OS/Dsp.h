#pragma once
// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DspTime.h"
#include "StereoSample.h"


namespace Dsp
{
extern StereoSample audio_out_buffer[];
extern StereoSample audio_in_buffer[];

extern bool	  audio_input_device_present;
extern bool	  audio_input_device_enabled;
extern bool	  audio_output_device_enabled;
extern Sample audio_output_volume;


extern void startCoreAudio(bool input_enabled); //, int playthrough_mode );
extern void stopCoreAudio();

inline bool isAudioInputDevicePresent() { return audio_input_device_present; }
inline bool isAudioInputDeviceEnabled() { return audio_input_device_enabled; }
extern void enableAudioInputDevice(bool f);

inline bool isAudioOutputDevicePresent() { return 1 /*yes*/; }
inline bool isAudioOutputDeviceEnabled() { return audio_output_device_enabled; }
extern void enableAudioOutputDevice(bool f);

extern void	  setOutputVolume(Sample volume);
inline Sample getOutputVolume() { return audio_output_volume; }

// extern	void	setPlaythrough				( uint mode );
//		enum
//		{		playthrough_minus10dB,
//				playthrough_minus30dB,
//			 	playthrough_off
//		};

extern void outputSamples(const StereoSample&, Time start, Time end);
extern void outputSamples(Sample, Time start, Time end);

// ---- Utilities ----

inline void shiftBuffer(StereoSample* bu)
{
	for (int i = 0; i < DSP_SAMPLES_STITCHING; i++) bu[i] = bu[DSP_SAMPLES_PER_BUFFER + i];
}
inline void clearBuffer(StereoSample* bu) // preserves stitching at buffer start
{
	for (int i = DSP_SAMPLES_STITCHING; i < DSP_SAMPLES_PER_BUFFER + DSP_SAMPLES_STITCHING; i++) bu[i] = 0.0;
}


} // namespace Dsp
