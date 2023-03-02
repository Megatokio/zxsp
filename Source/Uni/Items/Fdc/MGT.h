#pragma once
// Copyright (c) 2023 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc.h"


class MGT final : public Fdc
{
public:
	explicit MGT(Machine*);

protected:
	~MGT() override = default;
};
