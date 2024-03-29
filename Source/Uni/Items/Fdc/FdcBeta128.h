#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Fdc.h"


class FdcBeta128 final : public Fdc
{
public:
	explicit FdcBeta128(Machine*);

protected:
	~FdcBeta128() override = default;
};
