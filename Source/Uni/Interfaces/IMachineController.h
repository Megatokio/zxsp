// Copyright (c) 2023 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Templates/RCPtr.h"
#include "zxsp_types.h"
#include <memory>

class IMachineController
{
public:
	IMachineController() noexcept = default;
	virtual ~IMachineController() = default;

	virtual void memoryModified(Memory* m, uint how) volatile = 0;
	virtual void itemAdded(RCPtr<Item>) volatile			  = 0;
	virtual void itemRemoved(Item*) volatile				  = 0;
	virtual void showMessage(MessageStyle, cstr text)		  = 0;
};
