// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineList.h"
#include "OS/Dsp.h"

namespace gui
{

volatile MachineList machine_list;

void MachineList::runMachinesForSound(const StereoBuffer audio_in_buffer, StereoBuffer audio_out_buffer)
{
	for (uint i = 0; i < count(); i++)
	{
		volatile Machine* vm = data[i].get();
		if (vm->isPowerOn())
		{
			if (NVPtr<Machine> machine {vm, 50 * 1000}) // timeout = 50 µs
			{
				if (machine->isPowerOn())
				{
					if (machine->isRunning()) // not suspended
					{
						machine->runForSound(audio_in_buffer, audio_out_buffer, 0);
						if (machine->cpu_clock > 100000) continue;
					}

					machine->drawVideoBeamIndicator();
				}
			}
			else // !machine
			{
				if (debug) logline("runMachinesForSound: failed to lock");
				continue;
			}
		}
	}
}

} // namespace gui
