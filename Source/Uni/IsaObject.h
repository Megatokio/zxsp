#pragma once
// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "helpers.h"
#include "isa_id.h"
#include "kio/kio.h"
#include "zxsp_types.h"
class FD;


extern isa_id isa_pid[];   // parent id of id
extern cstr	  isa_names[]; // (default) item names


class IsaObject
{
public:
	const isa_id id;	 // precise isa_id for this item
	const isa_id grp_id; // major base class of this item, e.g. isa_Joy or isa_Ula
	cstr		 name;

protected:
	IsaObject(isa_id id, isa_id grp) : id(id), grp_id(grp), name(isa_names[id]) {}
	IsaObject(const IsaObject& q) = default;
	IsaObject(IsaObject&& q)	  = default;

	IsaObject& operator=(const IsaObject& q) = delete;
	IsaObject& operator=(IsaObject&& q)		 = delete;

public:
	virtual ~IsaObject() = default;

	bool isA(isa_id i) const volatile
	{
		isa_id j = id;
		while (j && j != i) { j = isa_pid[j]; }
		return i == j;
	}
	isa_id isaId() const volatile { return id; }
	isa_id grpId() const volatile { return grp_id; }
};


//===================================================================

// define safe casting procs:			e.g. Item* ItemPtr(object)

#define DEFPTR(ITEM)                                                 \
  inline ITEM* ITEM##Ptr(IsaObject* o)                               \
  {                                                                  \
	assert(!o || o->isA(isa_##ITEM));                                \
	return reinterpret_cast<ITEM*>(o);                               \
  }                                                                  \
                                                                     \
  inline const ITEM* ITEM##Ptr(const IsaObject* o)                   \
  {                                                                  \
	assert(!o || o->isA(isa_##ITEM));                                \
	return reinterpret_cast<const ITEM*>(o);                         \
  }                                                                  \
                                                                     \
  inline volatile ITEM* ITEM##Ptr(volatile IsaObject* o)             \
  {                                                                  \
	assert(!o || o->isA(isa_##ITEM));                                \
	return reinterpret_cast<volatile ITEM*>(o);                      \
  }                                                                  \
                                                                     \
  inline volatile const ITEM* ITEM##Ptr(volatile const IsaObject* o) \
  {                                                                  \
	assert(!o || o->isA(isa_##ITEM));                                \
	return reinterpret_cast<volatile const ITEM*>(o);                \
  }

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
