// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"


namespace zxsp
{

class IcTester : public Item
{
public:
	explicit IcTester(Machine*);

protected:
	~IcTester() override = default;
};

} // namespace zxsp
