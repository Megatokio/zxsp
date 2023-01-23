#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Mmu for +2A and +3.
	based on Mmu128k
*/

#include "Mmu128k.h"


class MmuPlus3 : public Mmu128k
{
	uint8	port_1ffd;

public:
	explicit MmuPlus3(Machine*);

	// set ports:
	// override mmu_is_locked
	void	setDiscMotor	(bool f)	{ setPort1ffd( f ? port_1ffd|0x08 : port_1ffd&~0x08 ); }
	void	setPrinterStrobe(bool f)	{ setPort1ffd( f ? port_1ffd|0x10 : port_1ffd&~0x10 ); }
	void	setRamOnlyMode	(bool f)	{ setPort1ffd( f ? port_1ffd|0x01 : port_1ffd&~0x01 ); }

	bool	getDiscMotorState() volatile const noexcept	{ return port_1ffd & 0x08; }
	bool	getPrinterStrobe() volatile const noexcept	{ return port_1ffd & 0x10; }
	bool	isRamOnlyMode	() volatile const noexcept	{ return port_1ffd & 0x01; }

protected:
	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	// Mmu interface:
	void    romCS(bool disable) override;		// from rear-item: daisy chain
	bool	hasPort7ffd() volatile const noexcept override	{ return yes; }
	bool	hasPort1ffd() volatile const noexcept override	{ return yes; }
	//bool	hasPortF4()	volatile const noexcept override	{ return no; }
	uint8	getPort1ffd() volatile const override			{ return port_1ffd; }
	void	setPort7ffd(uint8 n) override		{ set_port_7ffd_and_1ffd(n,port_1ffd); }
	void	setPort1ffd(uint8 n) override		{ set_port_7ffd_and_1ffd(port_7ffd,n); }

	// Mmu128 interface:
	void	setMmuLocked(bool f) override		{ setPort7ffd( f ? port_7ffd|0x20 : port_7ffd&~0x20 ); }
	uint    getPageC000() volatile const noexcept override;
	uint    getPage8000() volatile const noexcept override;
	uint    getPage4000() volatile const noexcept override;
	uint    getPage0000() volatile const noexcept override;

private:
	void	page_rom_plus3();
	void	page_ram_plus3();
	void	page_mem_plus3();
	void	page_only_ram();
	void	set_port_7ffd_and_1ffd(uint8,uint8);
};












