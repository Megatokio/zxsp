// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "ExternalRam.h"


namespace zxsp
{

class Jupiter16kRam : public ExternalRam
{
public:
	explicit Jupiter16kRam(Machine*);

protected:
	~Jupiter16kRam() override;
};

} // namespace zxsp
