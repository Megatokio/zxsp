#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


class Tc2068Joy : public Joy
{
public:
	Tc2068Joy(Machine* m, isa_id id = isa_Tc2068Joy) : Joy(m, id, internal, nullptr, nullptr, "J1", "J2") {}

	// Item interface:
	void input(Time, int32, uint16, uint8&, uint8&) {}
};


class Ts2068Joy : public Tc2068Joy
{
public:
	explicit Ts2068Joy(Machine* m) : Tc2068Joy(m, isa_Ts2068Joy) {}
};


class U2086Joy : public Tc2068Joy
{
public:
	explicit U2086Joy(Machine* m) : Tc2068Joy(m, isa_U2086Joy) {}
};
