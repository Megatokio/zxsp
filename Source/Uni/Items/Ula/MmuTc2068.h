#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Files/TccRom.h"
#include "MmuTc2048.h"


class MmuTc2068 : public MmuTc2048
{
	TccRom* cartridge;
	bool	exrom_selected;

	// bank controller registers:
	uint8 hold;
	uint8 bna;
	uint8 port_FD; // bank ctrl. cmd port

public:
	explicit MmuTc2068(Machine*, isa_id = isa_MmuTc2068);
	virtual ~MmuTc2068();

	void selectBusExpansionUnit(bool);

	TccRom* getCartridge() { return cartridge; }
	void	ejectCartridge();
	void	insertCartridge(cstr filepath);
	void	saveCartridgeAs(cstr fpath)
	{
		if (cartridge) cartridge->saveAs(fpath);
	}
	bool isLoaded() const volatile { return cartridge != nullptr; }

	bool isZxspEmu() const volatile
	{
		assert(isMainThread());
		return cartridge ? cartridge->isZxspEmu() : no;
	}
	TccRomId getTccId() const volatile
	{
		assert(isMainThread());
		return cartridge ? cartridge->getTccId() : TccUnknown;
	}
	cstr getFilepath() const volatile
	{
		assert(isMainThread());
		return cartridge ? cartridge->getFilepath() : nullptr;
	}

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	// Mmu interface:
	// bool	hasPortF4() volatile const noexcept	override { return yes; }
	// uint8	getPortF4() override						{ return port_F4; }
	void setPortF4(uint8 n) override { set_port_f4(n, 0xff); }
	void romCS(bool disable) override; // from rear-item: daisy chain

	// MmuTc2048 interface:
	void selectEXROM(bool) override;

private:
	void set_port_f4(uint8, uint8);
};
