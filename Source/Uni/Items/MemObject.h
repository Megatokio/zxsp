#pragma once
// Copyright (c) 2017 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "kio/kio.h"


// Helper to be used in ToolWindow as a 'virtual' item:


class MemObject : public IsaObject
{
public:
	MemObject(isa_id id) : IsaObject(id, id) {} // note: group == id
};
