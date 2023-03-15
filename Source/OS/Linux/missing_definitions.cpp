// Copyright (c) 2019 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Dsp.h"
#include "kio/kio.h"

Time	  system_time		 = 0.0;
Frequency samples_per_second = 44100;

namespace os
{

void startCoreAudio(bool input_enabled)
{
	(void)input_enabled;
	debugstr("startCoreAudio\n");
	TODO();
}

void stopCoreAudio() { debugstr("stopCoreAudio\n"); }
} // namespace os
