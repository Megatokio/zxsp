#pragma once
/*	Copyright  (c)	Günter Woigk 2004 - 2019
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

#include <QObject>
#include "kio/kio.h"
#include "zxsp_types.h"
#include "isa_id.h"
#include "helpers.h"
class FD;


extern isa_id	isa_pid[];		// parent id of id
extern cstr		isa_names[];	// (default) item names


class IsaObject : public QObject
{
public:
	const isa_id  id;		// precise isa_id for this item
	const isa_id  grp_id;	// major base class of this item, e.g. isa_Joy or isa_Ula
	cstr		  name;

protected:
	IsaObject(QObject* p, isa_id id, isa_id grp) :QObject(p),id(id),grp_id(grp),name(isa_names[id]){}
	IsaObject(QObject* p, const IsaObject& q)	 :QObject(p),id(q.id),grp_id(q.grp_id),name(q.name){}
	IsaObject(const IsaObject& q)				 :QObject(q.parent()),id(q.id),grp_id(q.grp_id),name(q.name){}
	IsaObject(const IsaObject&& q)				 :QObject(q.parent()),id(q.id),grp_id(q.grp_id),name(q.name){}

	IsaObject&	operator= (const IsaObject& q)	= delete;
	IsaObject&	operator= (const IsaObject&& q)	= delete;

public:
	virtual ~IsaObject() {}

	bool		isA		(isa_id i) volatile const	{isa_id j=id; while(j&&j!=i) { j=isa_pid[j]; } return i==j;}
	isa_id		isaId	() volatile const			{return id;}
	isa_id      grpId	() volatile const			{return grp_id;}

virtual void	saveToFile  (FD&) const throws;
virtual void	loadFromFile(FD&) throws;
};


//===================================================================

// define safe casting procs:			e.g. Item* ItemPtr(object)

#define DEFPTR(ITEM)	\
inline ITEM* ITEM ## Ptr(IsaObject*o) {					\
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<ITEM*>(o); }				\
\
inline const ITEM* ITEM ## Ptr(const IsaObject*o) {		\
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<const ITEM*>(o); }			\
\
inline volatile ITEM* ITEM ## Ptr(volatile IsaObject* o) { \
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<volatile ITEM*>(o); }		\
\
inline volatile const ITEM* ITEM ## Ptr(volatile const IsaObject* o) { \
	assert(!o||o->isA(isa_ ## ITEM));					\
	return reinterpret_cast<volatile const ITEM*>(o); }	\

DEFPTR(Machine)
DEFPTR(MachineZx80)
DEFPTR(MachineZx81)
DEFPTR(ZxspRenderer)
DEFPTR(MonoRenderer)
DEFPTR(ZxspGifWriter)
DEFPTR(MonoGifWriter)

DEFPTR(Item)
DEFPTR(TapeRecorder)
DEFPTR(ZonxBox)
DEFPTR(ZxPrinter)
DEFPTR(KempstonJoy)
DEFPTR(KempstonMouse)
DEFPTR(ZxIf1)
DEFPTR(ZxIf2)
DEFPTR(UlaZxsp)
DEFPTR(Ula128k)
DEFPTR(UlaPlus3)
DEFPTR(UlaTc2048)
DEFPTR(MmuTc2048)
DEFPTR(MmuTc2068)
DEFPTR(UlaMono)
DEFPTR(UlaZx80)
DEFPTR(UlaZx81)
DEFPTR(Crtc)
DEFPTR(Joy)
DEFPTR(Ay)
DEFPTR(Mmu)
DEFPTR(Ula)
DEFPTR(Z80)
DEFPTR(IcTester)
DEFPTR(Keyboard)
DEFPTR(Zx3kRam)
DEFPTR(MmuPlus3)
DEFPTR(Mmu128k)
DEFPTR(PrinterPlus3)
DEFPTR(Printer)
DEFPTR(FdcPlus3)
DEFPTR(Fdc)
DEFPTR(DivIDE)
DEFPTR(CurrahMicroSpeech)
DEFPTR(Memotech64kRam)
DEFPTR(SpectraVideo)
DEFPTR(Multiface)
DEFPTR(Multiface1)
DEFPTR(Multiface128)
DEFPTR(Multiface3)
DEFPTR(MachineTc2068)
DEFPTR(MachineZxPlus3)













