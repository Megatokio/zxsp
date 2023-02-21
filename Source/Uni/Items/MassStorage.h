#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "kio/kio.h"


class MassStorage : public Item
{
protected:
	MassStorage(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr);
	~MassStorage() override;
};
