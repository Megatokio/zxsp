#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaMono.h"
#include "TVDecoderMono.h"

class UlaZx81 : public UlaMono
{
public:
	explicit UlaZx81(Machine*);
	virtual ~UlaZx81() override;

// Item interface:
	virtual void powerOn (/*t=0*/ int32 cc) override;
	virtual void reset (Time t, int32 cc) override;
	virtual void input (Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	virtual void output (Time t, int32 cc, uint16 addr, uint8 byte) override;
	virtual void videoFrameEnd (int32 cc) override;

	virtual int32 doFrameFlyback(int32 cc) override;
	virtual void drawVideoBeamIndicator(int32 cc) override;

	virtual void setBorderColor(uint8) override		{ border_color = 0xFF; } // White. ZX81 has no "border color"

	virtual int32 cpuCycleOfInterrupt() override	{ return 0; }		// ZX81 has no regular timer interrupt
	virtual int32 cpuCycleOfIrptEnd() override		{ return 0; }
	virtual int32 cpuCycleOfNextCrtRead() override	{ return 1<<30; }	// ZXSP++ only

	virtual int32 cpuCycleOfFrameFlyback() override;
	virtual int32 updateScreenUpToCycle(int32 cc) override;
	virtual void  crtcRead(int32 cc, uint byte) override;
	virtual uint8 interruptAtCycle(int32, uint16) override;

	int32 nmiAtCycle(int32 cc_nmi);

	void set60Hz(bool=1) override;
	int32 getCcPerFrame() volatile const override	{ return cc_per_frame; }
	void setupTiming() override {}

	void enableMicOut(bool f);

	static constexpr uint waitmap_size = 207; // cc
	uint8* getWaitmap() { return u8ptr(waitmap); }

private:
	void mic_out(Time, Sample);

	TVDecoderMono tv_decoder;
	int32 cc_per_frame = 0;
	uint8 waitmap[waitmap_size];

	uint lcntr = 0;				// 3 bit line counter
	bool nmi_enabled = no;		// state of the NMI_enable flipflop
	bool sync = no;				// state of the combined sync output
	bool vsync = no;			// state of the vsync flipflop
	bool hsync = no;			// state of the hsync generator
	int32 cc_hsync_next = 16;	// next sheduled toggle

	int32 cc_sync_on = 0;
	int32 cc_frame_start = 0;

	bool vsync_enabled() const { return !nmi_enabled; }

	void set_sync(int32 cc, bool f);
	void run_hsync(int32 cc);
	void reset_hsync(int32 cc, int32 dur);
	void clear_vsync(int32 cc);
	void set_vsync(int32 cc);
	void clear_waitmap();
	void setup_waitmap(int32 cc_nmi);
	void disable_nmi(int32 cc);
	void enable_nmi(int32 cc);

	static constexpr int32 cc_per_line		= 207;
	static constexpr int32 cc_per_frame_50	= 310*207;	// std. ZX81 ROM, measured with zxsp!!!
	static constexpr int32 cc_per_frame_60	= 262*207;	// std. ZX81 ROM, measured with zxsp!!!
	static constexpr int32 cc_per_frame_min	= 240*207;
	static constexpr int32 cc_per_frame_max	= 340*207;
};

























