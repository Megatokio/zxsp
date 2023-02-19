#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "kio/kio.h"


class Crtc : public Item
{
	friend class Machine;

protected:
	const ZxInfo* info; // machine info
	gui::Screen*  screen;
	CoreByte*	  video_ram; // current video ram

	static constexpr int cc_per_byte = 4; // ula cycles per 8 pixels
	int					 lines_in_screen; // lines in active screen area
	int					 lines_before_screen;
	int					 lines_after_screen;
	int					 lines_per_frame;
	int					 columns_in_screen = 32 * 8;
	int					 cc_per_line;

	uint8 border_color; // current border color
	bool  is60hz;

public:
	uint8		 getBorderColor() const volatile { return border_color; }
	virtual void setBorderColor(uint8) {}
	CoreByte*	 getVideoRam() { return video_ram; }

	bool is60Hz() const volatile { return is60hz; }
	bool is50Hz() const volatile { return !is60hz; }

	int			  getLinesBeforeScreen() const volatile { return lines_before_screen; } // nominal
	int			  getLinesInScreen() const volatile { return lines_in_screen; }			// nominal
	int			  getLinesAfterScreen() const volatile { return lines_after_screen; }	// nominal
	int			  getLinesPerFrame() const volatile { return lines_per_frame; }
	int			  getColumnsInScreen() const volatile { return columns_in_screen; }
	int			  getCcPerByte() const volatile { return cc_per_byte; }					 // const
	int			  getCcPerLine() const volatile { return cc_per_line; }					 // nominal
	int			  getBytesPerLine() const volatile { return cc_per_line / cc_per_byte; } // nominal
	virtual int32 getCcPerFrame() const volatile { return lines_per_frame * cc_per_line; }

	void		  attachToScreen(gui::Screen*);
	virtual void  drawVideoBeamIndicator(int32 cc) = 0;
	virtual int32 doFrameFlyback(int32 cc)		   = 0;
	virtual int32 cpuCycleOfNextCrtRead()		   = 0;
	virtual int32 updateScreenUpToCycle(int32 cc)  = 0;
	virtual void  markVideoRam()				   = 0;
	virtual void  set60Hz(bool f = 1) { is60hz = f; }
	void		  set50Hz() { set60Hz(0); }

protected:
	Crtc(Machine*, isa_id, isa_id grp, Internal, cstr o_addr, cstr i_addr);

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;
};
