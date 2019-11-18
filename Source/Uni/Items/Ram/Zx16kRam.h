/*	Copyright  (c)	Günter Woigk 2012 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#ifndef ZX16KRAM_H
#define ZX16KRAM_H

#include "ExternalRam.h"


//  Speicher-Erweiterungen für den Sinclair ZX80 und ZX81
//  erweitert den ZX81 auf 16k

class Zx16kRam : public ExternalRam
{
public:
	explicit Zx16kRam(Machine*);
	virtual ~Zx16kRam();

protected:
	Zx16kRam(Machine*,isa_id);
};


class Memotech16kRam : public Zx16kRam
{
public:
	explicit Memotech16kRam(Machine* m)		:Zx16kRam(m,isa_Memotech16kRam){}
};


class Stonechip16kRam : public Zx16kRam		// TODO: konnte auf 32k erweitert werden.
{
public:
	explicit Stonechip16kRam(Machine* m)	:Zx16kRam(m,isa_Stonechip16kRam){}
};


class Ts1016Ram : public Zx16kRam			// Timex Sinclair
{
public:
	explicit Ts1016Ram(Machine* m)			:Zx16kRam(m,isa_Ts1016Ram){}
};


#endif















