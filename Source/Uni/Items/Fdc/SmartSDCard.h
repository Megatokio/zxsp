#pragma once
// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joystick.h" // physical joysticks
#include "MassStorage.h"
#include "Memory.h"
#include "Z80/Z80.h"
class OverlayJoystick;


class Sio
{
public:
	void output(Time, bool) {}
	bool input(Time) { return 0; }
	void audioBufferEnd(Time) {}
	void init() {}
};

class SDCard
{
public:
	void  output(uint8) {}
	uint8 input() { return 0; }
	void  init() {}
};

enum FlashCommandState {
	flash_writing,
	flash_idle,
	flash_AA,		 // 1st byte ($5555) = $AA received
	flash_AA55,		 // 2nd byte ($2AAA) = $55 received
	flash_AA55A0,	 // 3rd byte ($5555) = $A0 received
	flash_AA5580,	 // 3rd byte ($5555) = $80 received
	flash_AA5580AA,	 // 4th byte ($5555) = $AA received
	flash_AA5580AA55 // 5th byte ($2AAA) = $55 received
};


class SmartSDCard : public MassStorage
{
	MemoryPtr ram;
	MemoryPtr rom;

	Joystick*		 joystick;
	OverlayJoystick* overlay;
	SDCard*			 sd_card;
	Sio*			 sio;

	// i/o registers:
	// ram_config = config & 0x00FF
	// rom_config = config & 0xFF00
	uint16 config;

	static const int rampage_bits = 0x000F, // ram_config:
		sio_tx = 0x0010, sio_rx = 0x0010, aux_cs = 0x0020, sdcard_cs = 0x0040, ram_enabled = 0x0080,

					 rompage_bits = 0x0F00, // rom_config:
		pageout_armed = 0x4000, memory_disabled = 0x8000;

	// Dip switches:
	bool dip_joystick_enabled;
	bool dip_memory_enabled; // does not apply to flash write!
	bool dip_force_bank_B;
	bool dip_flash_write_enabled;

	// writing to Flash:
	FlashCommandState flash_state;
	bool			  flash_dirty;
	bool			  flash_software_id_mode;
	int32			  cc_flash_write_end; // during flash write
	uint8			  flash_byte_written; // byte seen during flash write

public:
	explicit SmartSDCard(Machine* m);
	virtual ~SmartSDCard();

	// DIP switches:
	void setMemoryEnabled(bool);
	void setForceBankB(bool);
	void enableFlashWrite(bool);

	// Joystick handling:
	void	   setJoystickEnabled(bool);
	void	   insertJoystick(int id) volatile;
	JoystickID getJoystickID() const volatile { return indexof(joystick); }

protected:
	// Item interface:
	void  powerOn(/*t=0*/ int32 cc) override;
	void  reset(Time t, int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	uint8 handleRomPatch(uint16 pc, uint8 o) override;					   // returns new opcode
	uint8 readMemory(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	void  writeMemory(Time t, int32 cc, uint16 addr, uint8 byte) override; // for memory mapped i/o
	// void	audioBufferEnd	(Time t) override;
	void videoFrameEnd(int32 cc) override;
	// void	triggerNmi		() override;
	// void	ramCS           (bool active) override;		// ZX80, ZX81
	// void	romCS           (bool active) override;		// ZX81, ZXSP, ZX128, +2; ROMCS1+ROMCS2: +2A, +3

private:
	void set_ram_config(Time t, uint);
	void set_rom_config(uint);
	void set_rom_pageout_bits(uint a = 0, uint e = 0xffff);
	void clear_rom_pageout_bits(uint a = 0, uint e = 0xffff);
	void finish_flash_write();
	void start_flash_write();
	void set_dip_flash_write_enabled(bool);
	void enable_flash_write();
	void disable_flash_write();
	void map_card_memory(uint a, uint e);
	bool memory_enabled() { return dip_memory_enabled && (~config & memory_disabled); }
	// bool	memory_disabled()			{ return !dip_memory_enabled || (config&memory_disabled); }
	int read_memory(int32 cc, uint16 pc, uint8 byte);
};
