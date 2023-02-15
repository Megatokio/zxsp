// Copyright (c) 2019 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Dsp.h"
#include "kio/kio.h"

namespace Dsp
{
void startCoreAudio(bool input_enabled)
{
	debugstr("startCoreAudio\n");
	TODO();
}
void stopCoreAudio() { debugstr("stopCoreAudio\n"); }
} // namespace Dsp


#include "Mouse.h"
Mouse mouse;
Mouse::Mouse() : dx(0), dy(0) { debugstr("Mouse\n"); }
Mouse::~Mouse() { debugstr("~Mouse"); }
void Mouse::grab(QWidget*) { debugstr("Mouse::grab\n"); }
void Mouse::ungrab() { debugstr("Mouase::ungrab\n"); }


#include "Joystick.h"
void findUsbJoysticks() { debugstr("findUsbJoysticks"); }


#include "audio/AudioDecoder.h"
AudioDecoder::AudioDecoder() { debugstr("AudioDecoder\n"); }
AudioDecoder::~AudioDecoder() { debugstr("~AudioDecoder"); }
void AudioDecoder::seekSamplePosition(uint32)
{
	debugstr("AudioDecoder::seekSamplePosition\n");
	TODO();
}
uint32 AudioDecoder::read(int16*, uint32 max_frames, uint num_channels)
{
	debugstr("AudioDecoder::read\n");
	TODO();
}
void AudioDecoder::open(cstr filename)
{
	debugstr("AudioDecoder::open\n");
	TODO();
}
