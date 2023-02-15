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

	VIR void setMmuLocked(bool f) { setPort7ffd(f ? port_7ffd | 0x20 : port_7ffd & ~0x20); }
	VIR uint getPageC000() const volatile noexcept { return port_7ffd & 7; }
	VIR uint getPage8000() const volatile noexcept { return 2; }
	VIR uint getPage4000() const volatile noexcept { return 5; }
	VIR uint getPage0000() const volatile noexcept { return (port_7ffd >> 4) & 1; } // except if ROMDIS
	uint	 getVideopage() const volatile noexcept { return port_7ffd & 8 ? 7 : 5; }
	bool	 port7ffdIsLocked() const volatile noexcept { return port_7ffd & 0x20; }

protected:
	Mmu128k(Machine*, isa_id, cstr o_addr, cstr i_addr);

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
	void saveToFile(FD&) const throws override;
	void loadFromFile(FD&) throws override;

	// Mmu interface:
	void  romCS(bool active) override; // from rear-item: daisy chain
	bool  hasPort7ffd() const volatile noexcept override { return yes; }
	uint8 getPort7ffd() const volatile override { return port_7ffd; }
	void  setPort7ffd(uint8) override;

private:
	void page_rom_128k();
	void page_ram_128k();
	void page_mem_128k();
};
