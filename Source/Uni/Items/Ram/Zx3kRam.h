// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "ExternalRam.h"


namespace zxsp
{

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

} // namespace zxsp
