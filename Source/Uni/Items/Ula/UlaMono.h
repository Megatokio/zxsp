#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ula.h"


class UlaMono : public Ula
{
protected:
	UlaMono(Machine* m, isa_id id, cstr oaddr, cstr iaddr) : Ula(m,id,oaddr,iaddr) {}

public:
	virtual void crtcRead(int32, uint) = 0;
};












