#pragma once
// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "helpers.h"
#include "isa_id.h"
#include "kio/kio.h"
#include "zxsp_types.h"


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
