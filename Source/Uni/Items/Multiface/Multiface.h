#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Memory.h"
#include "kio/kio.h"


class Multiface : public Item
{
protected:
	MemoryPtr rom;
	MemoryPtr ram;
	bool	  nmi_pending;
	bool	  paged_in;

	void page_in();
	void page_out();

	Multiface(Machine*, isa_id, cstr rom, cstr o_addr, cstr i_addr);

public:
	bool isNmiPending() const volatile { return nmi_pending; }
	bool isPagedIn() const volatile { return paged_in; }

protected:
	virtual ~Multiface() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
};
