/*	Copyright  (c)	Günter Woigk 2013 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/


#ifndef SPECTRAVIDEOINTERFACE_H
#define SPECTRAVIDEOINTERFACE_H


#include <QObject>
#include "Ula/Crtc.h"
#include "Mac/Joystick.h"
#include "Memory.h"
#include "unix/files.h"
class OverlayJoystick;


class SpectraVideo : public Crtc
{
public:
	bool		has_port_7ffd;			// SPECTRA zxsp or zx128k model
	bool		new_video_modes_enabled;
	uint8		port_7fdf;				// video mode, video output and z80 write page
	uint8		port_7ffd;				// shadow of zx128k mmu port
	bool		shadowram_ever_used;
	MemoryPtr	shadowram;

	Joystick*	joystick;				// Joystick
	OverlayJoystick* overlay;
	uint		port_254;				// border
	uint8		port_239;				// RS232
	uint8		port_247;				// RS232
	bool		rs232_enabled;			// RS232
	bool		joystick_enabled;		// Joystick
	bool		if1_rom_hooks_enabled;	// ROM
    MemoryPtr	rom;
    cstr        filepath;
	//bool		romdis_in;				// rear-side input state		--> Item
    bool		own_romdis_state;		// own state

// CRTC:
	int32		current_frame;			// counter, used for flash phase
	int32		ccx;					// next cc for reading from video ram
	uint8*		attr_pixel;				// screen attribute and pixel triples
	//uint		bytes_per_octet;		// triples = 3

	//IoInfo*	ioinfo;					--> Item
	//uint		ioinfo_count;			""
	//uint		ioinfo_size;			""
	uint8*		alt_attr_pixel;			// alternate data set
	uint		alt_ioinfo_size;
	IoInfo*		alt_ioinfo;
	//ScreenZxsp* screen;
	int			cc_per_side_border;		// cc_per_line - 32 * cc_per_byte
	int			cc_frame_end;			// lines_per_frame * cc_per_line
	int			cc_screen_start;		// lines_before_screen*cc_per_line

private:
	void		activate_hooks();
	void		deactivate_hooks();
	void		init_rom();
	void		map_shadow_ram();
	bool		get_flash_phase() const					{ return (current_frame>>4)&1; }
	void		setup_timing();
	bool		mmu_is_locked()	volatile const noexcept	{ return port_7ffd&0x20; }
	void		_reset();

public:
	explicit SpectraVideo(Machine* m);
	~SpectraVideo();


// Item interface:
	void	powerOn			(/*t=0*/ int32 cc)override;
	void	reset			(Time t, int32 cc)override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask)override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte)override;
	//void	audioBufferEnd	(Time t)override;
	//void	videoFrameEnd	(int32 cc)override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;


// ROM handling:
    cstr    getRomFilepath	()  volatile const		{ return filepath; }
    cstr    getRomFilename	()  volatile const		{ return basename_from_path(filepath); }
	bool	isRomInserted	()	volatile const		{ return rom.isnot(nullptr); }
	bool	isRomPagedIn	()	volatile const		{ return own_romdis_state; }

	void	activateRom		();
	void	deactivateRom	();
	void	insertRom		(cstr path);
	void	ejectRom		();
	void	setIF1RomHooksEnabled(bool);

	uint8	handleRomPatch	(uint16 pc, uint8 opcode)override;
	void	romCS			(bool f)override;

// RS232 handling:
	void	setRS232Enabled		(bool);
	void	setPort239			(Time t, uint8 byte);	// CTS and Mode
	void	setPort247			(Time t, uint8 byte);	// data

// Joystick handling:
	void	setJoystickEnabled	(bool);
	void	insertJoystick		(int id) volatile;
	JoystickID getJoystickID	()		 volatile const	{ return indexof(joystick); }

//CRTC:
	int32	cpuCycleOfNextCrtRead() override			{ return ccx; }
	int32	updateScreenUpToCycle	(int32 cc) override;
	int32	doFrameFlyback			(int32 cc) override;
	void	drawVideoBeamIndicator	(int32 cc) override;
	void	markVideoRam			() override;
	void	setPort7ffd				(uint8);

	bool	newVideoModesEnabled	() volatile			{ return new_video_modes_enabled; }
	void	enableNewVideoModes		(bool);

	void	setBorderColor			(uint8)override;
	uint8	getVideoMode			()			{ return port_7fdf; }
	void	setVideoMode			(uint8 m);
	void	setPort7fdf				(int32 cc, uint8);	// set video mode
};


#endif



























