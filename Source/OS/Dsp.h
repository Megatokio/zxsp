// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#pragma once
#include "zxsp_globals.h"

extern Frequency samples_per_second; // DSP-Konstante & Zeitbasis des Systems: samples/second
extern Time		 system_time;		 // Realzeit [seconds]


namespace os
{

extern void startCoreAudio(bool input_enabled);
extern void stopCoreAudio();
extern void enableAudioInputDevice(bool f);
extern void enableAudioOutputDevice(bool f);
extern void setOutputVolume(Sample volume);

} // namespace os
