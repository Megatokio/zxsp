#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"


class GrafPad : public Item
{
public:
	explicit GrafPad(Machine* m);

protected:
	~GrafPad() override = default;
};
