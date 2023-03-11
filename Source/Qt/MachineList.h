// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Machine.h"
#include "Templates/Array.h"
#include "cpp/cppthreads.h"


namespace gui
{

class MachineList : private Array<std::shared_ptr<volatile Machine>>
{
	using Array::cnt;
	using Array::data;
	PLock mutex;

public:
	void lock() volatile { mutex.lock(); }
	void unlock() volatile { mutex.unlock(); }

	void append(std::shared_ptr<volatile Machine> m) { Array::append(m); }
	void remove(std::shared_ptr<volatile Machine> m) { Array::remove(m); }
	void runMachinesForSound();
};


extern volatile MachineList machine_list;

} // namespace gui
