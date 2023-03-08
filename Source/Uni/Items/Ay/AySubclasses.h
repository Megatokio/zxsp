#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ay.h"


class AyForZx128 final : public Ay
{
public:
	explicit AyForZx128(Machine*);

protected:
	~AyForZx128() override = default;
};


class AyForTc2068 final : public Ay
{
	Tc2068Joy* const tc2068joy;

public:
	AyForTc2068(Machine*, Tc2068Joy*);

protected:
	~AyForTc2068() override = default;

	void  portAOutputValueChanged(Time, uint8) override; // notification from setRegister(…) for attached hardware
	uint8 getInputValueAtPortA(Time, uint16) override;	 // callback from Ay::input(…) to get values of port pins
};


class DidaktikMelodik final : public Ay
{
public:
	explicit DidaktikMelodik(Machine*);

protected:
	~DidaktikMelodik() override = default;
};


class ZaxonAyMagic final : public Ay
{
public:
	explicit ZaxonAyMagic(Machine*);

protected:
	~ZaxonAyMagic() override = default;
};


class ZonxBox final : public Ay
{
public:
	explicit ZonxBox(Machine*);

protected:
	~ZonxBox() override = default;
};


class ZonxBox81 final : public Ay
{
public:
	explicit ZonxBox81(Machine*);

protected:
	~ZonxBox81() override = default;
};
