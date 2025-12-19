// Copyright (c) 2013 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Item.h"
#include "Memory.h"
#include "Ula/Crtc.h"
#include "VideoData.h"
#include "unix/files.h"

namespace zxsp
{

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
	uint8	   port_254;			  // border
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
	UlaZxsp*	   ula {nullptr};
	ZxspVideoData* bucket {nullptr}; // the currently constructed video frame
	//CoreByte*	   video_ram {nullptr}; // current video ram
	int32 ccx			= 0; // next cc for reading from video ram
	int	  frame_counter = 0; // counter, used for flash phase


private:
	// TODO: move to Crtc? Ula defines them too
	static constexpr int cc_per_byte = 4; // ula cycles per 8 pixels

	int lines_in_screen; // lines in active screen area
	int lines_before_screen;
	int lines_after_screen;
	int lines_per_frame;
	int cc_per_line;
	int cc_before_screen;
	//int columns_in_screen = 32 * 8;

	int cc_per_side_border; // cc_per_line - 32 * cc_per_byte
	int cc_frame_end;		// lines_per_frame * cc_per_line
	int cc_screen_start;	// lines_before_screen*cc_per_line

	void activate_hooks();
	void deactivate_hooks();
	void init_rom();
	void map_shadow_ram();
	bool get_flash_phase() const { return (frame_counter >> 4) & 1; }
	void setup_timing();
	bool mmu_is_locked() const volatile noexcept { return port_7ffd & 0x20; }
	void _reset(int32 cc);
	void put_bucket(ZxspVideoData* bucket, int32 cc);
	void get_bucket();

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
	bool isRomInserted() const volatile { return const_cast<MemoryPtr&>(rom).get() != nullptr; }
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
	int32 updateScreenUpToCycle(int32 cc) override;
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	//CoreByte* getVideoRam() override { return video_ram; }
	void markVideoRam() override;
	void setPort7ffd(uint8);
	bool getFlashPhase() { return (frame_counter >> 4) & 1; }

	bool newVideoModesEnabled() volatile { return new_video_modes_enabled; }
	void enableNewVideoModes(bool);

	void  setBorderColor(uint8) override;
	uint8 getVideoMode() { return port_7fdf; }
	void  setVideoMode(uint8 m);
	void  setPort7fdf(int32 cc, uint8); // set video mode
};

} // namespace zxsp
