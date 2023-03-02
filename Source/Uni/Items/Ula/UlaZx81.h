#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaZx80.h"


class UlaZx81 : public UlaZx80
{
	friend class MachineZx81;

public:
	static constexpr uint waitmap_size = 207; // cc

	explicit UlaZx81(Machine*);
	uint8* getWaitmap() { return u8ptr(waitmap); }

protected:
	~UlaZx81() override;

	// Item interface:
	void  powerOn(/*t=0*/ int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void  videoFrameEnd(int32 cc) override;
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	int32 updateScreenUpToCycle(int32 cc) override;
	void  crtcRead(int32 cc, uint byte) override;
	uint8 interruptAtCycle(int32, uint16) override;
	int32 nmiAtCycle(int32 cc_nmi);

private:
	uint8 waitmap[waitmap_size];
	// uint lcntr    = 0;  // 3 bit line counter
	bool nmi_enabled = no; // state of the NMI_enable flipflop
	bool sync		 = no; // state of the combined sync output
	// bool vsync    = no; // state of the vsync flipflop
	bool				   hsync		   = no; // state of the hsync generator
	int32				   cc_hsync_next   = 16; // next sheduled toggle
	static constexpr int32 cc_hsync_period = 207;

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
};
