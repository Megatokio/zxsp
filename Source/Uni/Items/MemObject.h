// Copyright (c) 2017 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "IsaObject.h"
#include "kio/kio.h"


// Helper to be used in ToolWindow as a 'virtual' item:


namespace zxsp
{

class MemObject : public IsaObject
{
public:
	MemObject(isa_id id) : IsaObject(id, id) {} // note: group == id
};

} // namespace zxsp
