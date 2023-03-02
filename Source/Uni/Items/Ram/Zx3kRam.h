#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"


class Zx3kRam : public ExternalRam
{
	uint size;

public:
	explicit Zx3kRam(Machine*, uint sz = 0);

	void setRamSize(uint);
	uint getRamSize() const volatile { return size; }

protected:
	~Zx3kRam() override;
};
