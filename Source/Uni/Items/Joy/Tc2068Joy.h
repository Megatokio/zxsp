#pragma once
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

#include "Joy.h"


class Tc2068Joy : public Joy
{
public:
	Tc2068Joy(Machine* m, isa_id id=isa_Tc2068Joy)	:Joy(m,id,internal,NULL,NULL,"J1","J2"){}

	// Item interface:
	void input(Time,int32,uint16,uint8&,uint8&)	{}
};


class Ts2068Joy : public Tc2068Joy
{
public:
	explicit Ts2068Joy(Machine* m)		:Tc2068Joy(m,isa_Ts2068Joy){}
};


class U2086Joy : public Tc2068Joy
{
public:
	explicit U2086Joy(Machine* m)		:Tc2068Joy(m,isa_U2086Joy){}
};




