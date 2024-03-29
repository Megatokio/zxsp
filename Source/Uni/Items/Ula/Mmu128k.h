#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Mmu.h"


class Mmu128k : public Mmu
{
protected:
	uint8 port_7ffd;

public:
	explicit Mmu128k(Machine*);

	virtual void setMmuLocked(bool f) { setPort7ffd(f ? port_7ffd | 0x20 : port_7ffd & ~0x20); }
	virtual uint getPageC000() const volatile noexcept { return port_7ffd & 7; }
	virtual uint getPage8000() const volatile noexcept { return 2; }
	virtual uint getPage4000() const volatile noexcept { return 5; }
	virtual uint getPage0000() const volatile noexcept { return (port_7ffd >> 4) & 1; } // except if ROMDIS
	uint		 getVideopage() const volatile noexcept { return port_7ffd & 8 ? 7 : 5; }
	bool		 port7ffdIsLocked() const volatile noexcept { return port_7ffd & 0x20; }

protected:
	Mmu128k(Machine*, isa_id, cstr o_addr, cstr i_addr);
	~Mmu128k() override = default;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;

	// Mmu interface:
	void romCS(bool active) override; // from rear-item: daisy chain
	void setPort7ffd(uint8) override;

public:
	bool  hasPort7ffd() const volatile noexcept override { return yes; }
	uint8 getPort7ffd() const volatile override { return port_7ffd; }

private:
	void page_rom_128k();
	void page_ram_128k();
	void page_mem_128k();
};
