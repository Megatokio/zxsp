#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"


//  Memory Extension for the Sinclair ZX80 and ZX81.
//  Extend memory to 16k

class Zx16kRam : public ExternalRam
{
public:
	explicit Zx16kRam(Machine*);

protected:
	Zx16kRam(Machine*, isa_id);
	~Zx16kRam() override;
};


class Memotech16kRam : public Zx16kRam
{
public:
	explicit Memotech16kRam(Machine* m) : Zx16kRam(m, isa_Memotech16kRam) {}

protected:
	~Memotech16kRam() override = default;
};


class Stonechip16kRam : public Zx16kRam // TODO: konnte auf 32k erweitert werden.
{
public:
	explicit Stonechip16kRam(Machine* m) : Zx16kRam(m, isa_Stonechip16kRam) {}

protected:
	~Stonechip16kRam() override = default;
};


class Ts1016Ram : public Zx16kRam // Timex Sinclair
{
public:
	explicit Ts1016Ram(Machine* m) : Zx16kRam(m, isa_Ts1016Ram) {}

protected:
	~Ts1016Ram() override = default;
};
