#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy.h"


class SinclairJoy : public Joy
{
protected:
	SinclairJoy(Machine*, isa_id, Internal internal, cstr i_addr="----.----.----.---0");

	// Item interface:
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;

public:
	// calc Sinclair 1/2 keyboard byte for joystick() byte:
	// note: keyboard bits are active-low and oK
  static uint8 calcS1ForJoy(uint8 joy)	// %000FUDLR active high -> %000FUDRL active low (54321)
	{ return uint8(~((joy&0x1cu) | ((joy&0x02u)>>1) | ((joy&0x01u)<<1))); }

  static uint8 calcS2ForJoy(uint8 joy)	// %000FUDLR active high -> %000LRDUF active low (67890)
	{ return uint8(~(((joy&0x10u)>>4) | ((joy&0x08u)>>2) | (joy&0x04u) | ((joy&0x03u)<<3))); }
};


class ZxPlus2Joy : public SinclairJoy
{
public:
	explicit ZxPlus2Joy(Machine* m)			:SinclairJoy(m,isa_ZxPlus2Joy,internal){}
};


class ZxPlus2AJoy : public SinclairJoy
{
public:
	explicit ZxPlus2AJoy(Machine* m)		:SinclairJoy(m,isa_ZxPlus2AJoy,internal){}
};


class ZxPlus3Joy : public SinclairJoy
{
public:
	explicit ZxPlus3Joy(Machine* m)			:SinclairJoy(m,isa_ZxPlus3Joy,internal){}
};


class Tk90xJoy : public SinclairJoy
{
	//	TK90X:
	//	Anscheinend wurden beide Joysticks über den einen Port herausgeführt:
	//	primär rechter Joystick (67890) mit COMMON an Pin 8
	//	über Adapter auch linker Joystick (12345) mit COMMON an Pin 7.

public:
	explicit Tk90xJoy(Machine* m)			:SinclairJoy(m,isa_Tk90xJoy,internal){}
};


class Tk95Joy : public SinclairJoy
{
	//	TK95:
	//	Anscheinend wie TK90X

public:
	explicit Tk95Joy(Machine* m)			:SinclairJoy(m,isa_Tk95Joy,internal){}
};










