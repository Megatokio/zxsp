#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Crtc.h"
#include "Item.h"
#include "Keymap.h"
#include "StereoSample.h"


class Ula : public Crtc
{
protected:
	uint8 ula_out_byte; // last out byte to ula: border, beeper, ear_out

	// Beeper/tape out:
	Sample beeper_volume;		  // 0.0 ... 1.0
	Sample beeper_current_sample; // current beeper elongation
	Time   beeper_last_sample_time;

public:
	Keymap keymap; // keyboard matrix as seen by ula

protected:
	Ula(Machine*, isa_id, cstr o_addr, cstr i_addr);
	~Ula() override = default;

	uint8		 readKeyboard(uint16 addr); // read bits from keyboard matrix
	virtual void setupTiming() = 0;

public:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void	reset			(Time t, int32 cc) override;
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
};
