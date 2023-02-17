#pragma once
// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaZxsp.h"
#include "zxsp_types.h"


class Ula128k : public UlaZxsp
{
protected:
	uint8 port_7ffd; // evtl. only bit 0x08 is valid

public:
	explicit Ula128k(Machine*);

	virtual void setPort7ffd(uint8);

protected:
	Ula128k(Machine*, isa_id, cstr o_addr, cstr i_addr);

	bool  mmu_is_locked() const volatile noexcept { return port_7ffd & 0x20; }
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void  powerOn(/*t=0*/ int32 cc) override;
	void  reset(Time t, int32 cc) override;
	void  markVideoRam() override;
	int32 addWaitCycles(int32 cc, uint16 addr) const volatile override;
};
