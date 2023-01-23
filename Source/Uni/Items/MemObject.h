#pragma once
// Copyright (c) 2017 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "IsaObject.h"


// Helper to be used in ToolWindow as a 'virtual' item:


class MemObject : public IsaObject
{
public:
	MemObject(QObject* p, isa_id id) : IsaObject(p,id,id) {}		// note: group == id
};



