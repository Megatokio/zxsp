#pragma once
// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Memory.h"
#include "Ula/Crtc.h"
#include "unix/files.h"


class SpectraVideo : public Crtc
{
	friend class Machine;

public:
	bool	  has_port_7ffd; // SPECTRA zxsp or zx128k model
	bool	  new_video_modes_enabled;
	uint8	  port_7fdf; // video mode, video output and z80 write page
	uint8	  port_7ffd; // shadow of zx128k mmu port
	bool	  shadowram_ever_used;
	MemoryPtr shadowram;

	JoystickID joystick_id;			  // Joystick
	uint	   port_254;			  // border
	uint8	   port_239;			  // RS232
	uint8	   port_247;			  // RS232
	bool	   rs232_enabled;		  // RS232
	bool	   joystick_enabled;	  // Joystick
	bool	   if1_rom_hooks_enabled; // ROM
	MemoryPtr  rom;
	cstr	   filepath;
	// bool romdis_in;	   // rear-side input state --> Item
	bool own_romdis_state; // own state

	// CRTC:
	int32  current_frame; // counter, used for flash phase
	int32  ccx;			  // next cc for reading from video ram
	uint8* attr_pixel;	  // screen attribute and pixel triples

	// IoInfo*	ioinfo;					--> Item
	// uint		ioinfo_count;			""
	// uint		ioinfo_size;			""
	uint8*	alt_attr_pixel; // alternate data set
	uint	alt_ioinfo_size;
	IoInfo* alt_ioinfo;
	int		cc_per_side_border; // cc_per_line - 32 * cc_per_byte
	int		cc_frame_end;		// lines_per_frame * cc_per_line
	int		cc_screen_start;	// lines_before_screen*cc_per_line

private:
	void activate_hooks();
	void deactivate_hooks();
	void init_rom();
	void map_shadow_ram();
	bool get_flash_phase() const { return (current_frame >> 4) & 1; }
	void setup_timing();
	bool mmu_is_locked() const volatile noexcept { return port_7ffd & 0x20; }
	void _reset();

public:
	// same as in file .z80:
	enum DipSwitches { EnableNewVideoModes = 1, EnableRs232 = 2, EnableJoystick = 4, EnableIf1RomHooks = 8 };

	explicit SpectraVideo(Machine* m, uint dip_switches);

protected:
	~SpectraVideo() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;

	uint8 getJoystickButtonsFUDLR();

public:
	// ROM handling:
	cstr getRomFilepath() const volatile { return filepath; }
	cstr getRomFilename() const volatile { return basename_from_path(filepath); }
	bool isRomInserted() const volatile { return rom; }
	bool isRomPagedIn() const volatile { return own_romdis_state; }

	void activateRom();
	void deactivateRom();
	void insertRom(cstr path);
	void ejectRom();
	void setIF1RomHooksEnabled(bool);

	uint8 handleRomPatch(uint16 pc, uint8 opcode) override;
	void  romCS(bool f) override;

	// RS232 handling:
	void setRS232Enabled(bool);
	void setPort239(Time t, uint8 byte); // CTS and Mode
	void setPort247(Time t, uint8 byte); // data

	// Joystick handling:
	void	   setJoystickEnabled(bool);
	void	   insertJoystick(JoystickID id) volatile { joystick_id = id; }
	JoystickID getJoystickID() const volatile { return joystick_id; }
	cstr	   getIdf() const volatile { return "K"; } // Kempston joystick
	uint8	   peekJoystickButtonsFUDLR() const volatile;
	bool	   isJoystickEnabled() const volatile { return joystick_enabled; }

	// CRTC:
	int32 cpuCycleOfNextCrtRead() override { return ccx; }
	int32 updateScreenUpToCycle(int32 cc) override;
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	void  markVideoRam() override;
	void  setPort7ffd(uint8);

	bool newVideoModesEnabled() volatile { return new_video_modes_enabled; }
	void enableNewVideoModes(bool);

	void  setBorderColor(uint8) override;
	uint8 getVideoMode() { return port_7fdf; }
	void  setVideoMode(uint8 m);
	void  setPort7fdf(int32 cc, uint8); // set video mode
};
