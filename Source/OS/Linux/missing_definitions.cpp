
#include "kio/kio.h"
#include "Dsp.h"

namespace Dsp{
StereoSample	audio_out_buffer [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];
StereoSample	audio_in_buffer	 [DSP_SAMPLES_PER_BUFFER+DSP_SAMPLES_STITCHING];
void startCoreAudio (bool input_enabled) {debugstr("startCoreAudio\n");TODO();}
void stopCoreAudio ( ) {debugstr("stopCoreAudio\n");}
void outputSamples ( Sample, Time start, Time end ) {}
void outputSamples ( cStereoSample&, Time start, Time end ) {}
}


#include "DspTime.h"

Frequency	samples_per_second;	// DSP-Konstante & Zeitbasis des Systems: samples/second
Time		system_time;		// Realzeit [seconds]


#include "Joystick.h"

Joystick* joysticks[max_joy];
void findUsbJoysticks() {debugstr("findUsbJoysticks\n");TODO();}


#include "Mouse.h"

Mouse mouse;
Mouse::Mouse(){debugstr("Mouse\n");}
Mouse::~Mouse(){debugstr("");}
void Mouse::grab(QWidget*){debugstr("Mouse::grab\n");}
void Mouse::ungrab(){debugstr("Mouase::ungrab\n");}


#include "audio/AudioDecoder.h"
AudioDecoder::AudioDecoder(){debugstr("AudioDecoder\n");}
AudioDecoder::~AudioDecoder(){debugstr("");}
void AudioDecoder::seekSamplePosition(uint32){debugstr("AudioDecoder::seekSamplePosition\n");TODO();}
uint32 AudioDecoder::read(int16*, uint32 max_frames, uint num_channels) {debugstr("AudioDecoder::read\n");TODO();}
void AudioDecoder::open(cstr filename) {debugstr("AudioDecoder::open\n");TODO();}


