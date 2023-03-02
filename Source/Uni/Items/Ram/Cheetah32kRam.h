#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"


class Cheetah32kRam : public ExternalRam
{
public:
	explicit Cheetah32kRam(Machine*);

protected:
	virtual ~Cheetah32kRam() override;
};
