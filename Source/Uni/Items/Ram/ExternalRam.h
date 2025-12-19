// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"
#include "zxsp_types.h"


namespace zxsp
{

class ExternalRam : public Item
{
protected:
	ExternalRam(Machine*, isa_id);
	~ExternalRam() override = default;
};

} // namespace zxsp
