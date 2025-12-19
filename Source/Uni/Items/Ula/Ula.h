// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Crtc.h"
#include "Item.h"
#include "Keymap.h"
#include "StereoSample.h"
#include "VideoData.h"

namespace zxsp
{

class Ula : public Crtc
{
	friend class Machine;
	friend class TVDecoderMono;

protected:
	uint8 ula_out_byte = 0; // last out byte to ula: border, beeper, ear_out

	// Beeper/tape out:
	Sample beeper_volume;		  // 0.0 ... 1.0
	Sample beeper_current_sample; // current beeper elongation
	Time   beeper_last_sample_time;

public:
	Keymap keymap; // keyboard matrix as seen by ula

protected:
	Ula(Machine*, isa_id, cstr o_addr, cstr i_addr);
	~Ula() override;

	uint8		 readKeyboard(uint16 addr); // read bits from keyboard matrix
	virtual void setupTiming() = 0;

public:
	//uint8	  getBorderColor() const volatile override { return ula_out_byte & 7; }
	//void	  setBorderColor(uint8 b) override = 0;
	//CoreByte* getVideoRam() override { return video_ram; }
	//VideoDataReceiver* getScreen(); //TODO eliminate { return screen; }

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

	virtual void set60Hz(bool f = 1);
	void		 set50Hz() { set60Hz(0); }

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void reset(Time t, int32 cc) override;
	// void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	// void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void audioBufferEnd(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	Sample getBeeperVolume() { return beeper_volume; }
	void   setBeeperVolume(Sample);

	void setLinesBeforeScreen(int n);
	void setLinesAfterScreen(int n);
	void setCcPerLine(int n);
	void setBytesPerLine(int n);

	virtual uint8 getFloatingBusByte(int32 /*cc*/) { return 0xff; }
	virtual int32 addWaitCycles(int32 cc, uint16 /*addr*/) const volatile { return cc; }
	virtual int32 cpuCycleOfInterrupt()	   = 0;
	virtual int32 cpuCycleOfIrptEnd()	   = 0;
	virtual int32 cpuCycleOfFrameFlyback() = 0; // when next ffb irpt
	virtual uint8 interruptAtCycle(int32, uint16) { return 0xff; /*RST_38*/ }

	virtual bool  hasPortFF() const volatile noexcept { return no; }
	virtual void  setPortFF(uint8) {}
	virtual uint8 getPortFF() const volatile { return 0xFF; }

	// helper for snapshot loader:
	void set_ula_out_byte(uint8 b) noexcept { ula_out_byte = b; }

protected:
	const ZxInfo* info; // machine info

	static constexpr int cc_per_byte = 4; // ula cycles per 8 pixels
	int					 lines_in_screen; // lines in active screen area
	int					 lines_before_screen;
	int					 lines_after_screen;
	int					 lines_per_frame;
	int					 columns_in_screen = 32 * 8;
	int					 cc_per_line;
	int					 cc_before_screen;

	bool is60hz;
};


//
// ----------------------------------
//		Inline Implementations
// ----------------------------------
//

} // namespace zxsp

/*



























*/
