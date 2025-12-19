// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"


namespace zxsp
{

class ZxIf1 : public Item
{
public:
	explicit ZxIf1(Machine*);

	bool isRomPagedIn() const { return no; }

protected:
	~ZxIf1() override = default;
};

} // namespace zxsp
