// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"


namespace zxsp
{

class AmxMouse : public Item
{
public:
	explicit AmxMouse(Machine*);

protected:
	~AmxMouse() override = default;
};

} // namespace zxsp
