// Copyright (c) 2015 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"
#include "kio/kio.h"


namespace zxsp
{

class MassStorage : public Item
{
protected:
	MassStorage(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr);
	~MassStorage() override;
};

} // namespace zxsp
