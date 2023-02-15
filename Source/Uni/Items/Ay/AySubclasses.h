#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ay.h"


class AyForZx128 : public Ay
{
public:
	explicit AyForZx128(Machine*);
};


class AyForTc2068 : public Ay
{
public:
	explicit AyForTc2068(Machine*);

	static uint8 ayByteForJoystickByte(uint8 joy);

protected:
	void  portAOutputValueChanged(Time, uint8); // notification from setRegister(…) for attached hardware
	uint8 getInputValueAtPortA(Time, uint16);	// callback from Ay::input(…) to get values of port pins
};


class DidaktikMelodik : public Ay
{
public:
	explicit DidaktikMelodik(Machine*);
};


class ZaxonAyMagic : public Ay
{
public:
	explicit ZaxonAyMagic(Machine*);
};


class ZonxBox : public Ay
{
public:
	explicit ZonxBox(Machine*);
};


class ZonxBox81 : public Ay
{
public:
	explicit ZonxBox81(Machine*);
};
