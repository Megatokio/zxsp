#pragma once
// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IdeDevice.h"
#include "MassStorage.h"
#include "Uni/Memory.h"
#include "unix/files.h"


class DivIDE : public MassStorage
{
	MemoryPtr rom;
	MemoryPtr ram;

	IdeDevice* cf_card;

	uint8 ide_data_out_latch; // 0 = write to latch, 1=write to IDE
	uint8 ide_data_in_latch;  // 0 = read from IDE,  1=read from latch
	bool  ide_data_latch_state;

	uint8 control_register;
	bool  jumper_E;
	bool  jumper_A;
	// bool	romdis_in;	   // rear-side input state
	bool auto_paged_in;	   // auto page-in active?
	bool own_romdis_state; // own state = auto_paged_in + CONMEM bit
	cstr romfilepath;

public:
	static constexpr cstr default_rom_name = "ESXdos 0.8.5";
	static constexpr cstr default_rom_path = "Roms/esxdos085.rom";
	static constexpr cstr default_rom	   = nullptr;

	DivIDE(Machine*, uint ramsize, cstr romfile = default_rom);
	~DivIDE() override;

	// ROM handling:
	cstr		 getRomFilepath() const volatile { return romfilepath; }
	cstr		 getRomFilename() const volatile { return basename_from_path(romfilepath); }
	bool		 isRomPagedIn() const volatile { return own_romdis_state; }
	cstr /*err*/ insertRom(cstr path);
	cstr /*err*/ insertDefaultRom() { return insertRom(default_rom); }
	void		 saveRom(FD&);

	// Disk handling:
	bool isDiskInserted() const volatile { return cf_card != nullptr; }
	void insertDisk(cstr path);
	void ejectDisk();

	// Memory and Registers:
	void	  setRamSize(uint);
	MemoryPtr getRam() { return ram; }
	MemoryPtr getRom() { return rom; }
	void	  setJumperE(bool); // 1 => rom paging enabled & eeprom write protected
	bool	  getJumperE() const volatile { return jumper_E; }
	uint8	  getCtrlRegister() const volatile { return control_register; }
	bool	  getMapRam() const volatile { return control_register & 0x40; }
	// bool		getIdeBusy		(Time t)			{ return cf_card && cf_card->isBusy(t); }

	// for Inspector:
	bool getIdeBusy() const volatile
	{
		assert(isMainThread());
		return cf_card && cf_card->is_busy();
	}
	bool isDiskWritable() const volatile
	{
		assert(isMainThread());
		return cf_card && cf_card->isWritable();
	}
	cstr	getDiskFilename() const volatile;
	Memory& getRam() const volatile
	{
		assert(isMainThread());
		return const_cast<MemoryPtr&>(ram).ref();
	}
	Memory& getRom() const volatile
	{
		assert(isMainThread());
		return const_cast<MemoryPtr&>(rom).ref();
	}
	// cstr	getDiskFilepath	() volatile const { assertMainThread(); return cf_card?cf_card->getFilepath():nullptr; }
	void setDiskWritable(bool) volatile;

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time, int32 cc) override;
	void input(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time, int32 cc, uint16 addr, uint8 byte) override;
	void audioBufferEnd(Time) override;
	// void	videoFrameEnd	(int32 cc);
	uint8 handleRomPatch(uint16, uint8) override;
	void  romCS(bool f) override;

private:
	bool mapram_is_set() { return control_register & 0x40; }
	bool conmem_is_set() { return control_register & 0x80; }
	uint mapped_rampage() { return control_register & ((ram.count() >> 13) - 1); }
	void mapMemory();
	void applyRomPatches();
};
