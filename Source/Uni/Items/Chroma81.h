// Copyright (c) 2026 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"
#include "Memory.h"
#include "Ula/UlaZx81.h"
#include "VideoData.h"
#include "unix/files.h"


namespace zxsp
{

class Chroma81 final : public Crtc
{
	friend class Chroma81Inspector;

public:
	enum DipSwitches {
		Enable16kRamAt4000				= 1,
		EnableWRXGraphics				= 2,
		Enable8kRamAt2000				= 4,
		EnableQSCharBoard				= 8, // and 1k RAM at $8400
		EnableRs232						= 16,
		Enable16kRamAtC000AndColorModes = 32, // and 16k RAM at $C000
	};

	Chroma81(Machine* m, uint dip_switches);

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;

	// CRTC:
	int32 updateScreenUpToCycle(int32 cc) override;
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	void  crtcRead(int32 cc, uint pc, uint byte) override;
	//void  markVideoRam() {}
	//void  setPort7ffd(uint8);

	//void enableColour(bool);
	//bool colourEnabled() volatile { return colour_enabled; }

	void  setBorderColor(uint8) override;
	void  setColourMode(uint8 m);
	uint8 getColourMode(); // { return port_7fdf; }
	//void  setPort7fdf(int32 cc, uint8); // set video mode

	// Joystick handling:
	void	   insertJoystick(JoystickID id) volatile { joystick_id = id; }
	JoystickID getJoystickID() const volatile { return joystick_id; }
	cstr	   getIdf() const volatile { return "C"; } // Cursor Key joystick
	uint8	   peekJoystickButtonsFUDLR() const volatile;
	uint8	   getJoystickButtonsFUDLR();

	// ROM handling:
	void insertRom(cstr path); //TODO
	void ejectRom();		   //TODO
	cstr getRomFilepath() const volatile { return rom_filepath; }
	cstr getRomFilename() const volatile { return basename_from_path(rom_filepath); }
	bool isRomInserted() const volatile { return const_cast<MemoryPtr&>(rom).get() != nullptr; }
	//bool isRomPagedIn() const volatile { return own_romdis_state; }
	//void activateRom();
	//void deactivateRom();

	// DIP switches:
	void setRS232Enabled(bool);
	void set8kRamAt2000Enabled(bool);
	void set16kRamAt4000Enabled(bool);
	void setQSEnabled(bool);
	void setWRXEnabled(bool);
	void setRamAtC000AndColorEnabled(bool);

private:
	~Chroma81() override;

	UlaZx81*   ula;
	bool	   dip_rs232_enabled;
	bool	   dip_ram_at_2000;					  // 8k
	bool	   dip_ram_at_4000;					  // 16k
	bool	   dip_ram_at_C000_and_color_enabled; // DIP switch
	bool	   dip_qs_enabled;
	bool	   dip_wrx_enabled;
	bool	   color_modes_enabled;
	bool	   specci_color_mode; // 0=colorized chars, 1=specci screen attributes
	JoystickID joystick_id;		  // Joystick
	MemoryPtr  ram;				  // Chroma ram
	MemoryPtr  rom;				  // cartridge
	uint8*	   waitmap;
	uint	   waitmap_size;
	cstr	   rom_filepath; // cartridge
	// bool ramdis_in;	    // rear-side input state --> Item
	// bool own_ramdis_state; // own state

	void map_memory();
};

} // namespace zxsp
