#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc.h"


class FdcD80 final : public Fdc
{
public:
	explicit FdcD80(Machine*);

protected:
	~FdcD80() override = default;
};
