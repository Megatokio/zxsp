// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MachineList.h"

namespace gui
{

volatile MachineList machine_list;

void MachineList::runMachinesForSound()
{
	for (uint i = 0; i < count(); i++)
	{
		NVPtr<Machine> machine(data[i].get(), 50 * 1000); // timeout = 50 µs

		if (machine->isPowerOn())
		{
			if (machine->isRunning()) // not suspended
			{
				machine->runForSound();
				if (machine->cpu_clock > 100000) continue;
			}

			machine->drawVideoBeamIndicator();
		}
	}
}

} // namespace gui
