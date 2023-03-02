#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"


class ZxIf1 : public Item
{
public:
	explicit ZxIf1(Machine*);

	bool isRomPagedIn() const { return no; }

protected:
	~ZxIf1() override = default;
};
