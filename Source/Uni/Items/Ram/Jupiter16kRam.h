#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"


class Jupiter16kRam : public ExternalRam
{
public:
	explicit Jupiter16kRam(Machine*);
	virtual ~Jupiter16kRam();
};














