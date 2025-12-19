// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"


namespace zxsp
{

class GrafPad : public Item
{
public:
	explicit GrafPad(Machine* m);

protected:
	~GrafPad() override = default;
};

} // namespace zxsp
