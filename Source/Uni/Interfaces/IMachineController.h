// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include <memory>
class Memory;
class Item;


class IMachineController
{
public:
	IMachineController() {}
	virtual ~IMachineController() {}

	virtual void memoryModified(Memory* m, uint how) volatile = 0;
	virtual void machineSuspendStateChanged() volatile		  = 0;
	virtual void rzxStateChanged() volatile					  = 0;
	virtual void itemAdded(std::shared_ptr<Item>) volatile	  = 0;
	virtual void itemRemoved(Item*) volatile				  = 0;
};
