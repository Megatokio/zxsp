#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "zxsp_types.h"


class ExternalRam : public Item
{
protected:
	ExternalRam(Machine*, isa_id);
};
