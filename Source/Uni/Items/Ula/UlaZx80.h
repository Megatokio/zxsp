// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "UlaMono.h"
#include "TVDecoderMono.h"


class UlaZx80 : public UlaMono
{
public:
	explicit UlaZx80(Machine*);
	virtual ~UlaZx80() override;

// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	void	videoFrameEnd	(int32 cc) override			{ tv_decoder.shiftCcTimeBase(cc); }
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	int32	doFrameFlyback(int32 cc) override			{ return tv_decoder.doFrameFlyback(cc); }
	void	drawVideoBeamIndicator(int32 cc) override	{ tv_decoder.drawVideoBeamIndicator(cc); }

	void	setBorderColor(uint8) override { border_color = 0xFF; }		// White: ZX80 has no "border color"

	int32	cpuCycleOfInterrupt() override				{ return 0; }	// ZX80 has no regular timer interrupt
	int32	cpuCycleOfIrptEnd() override				{ return 0; }
	int32	cpuCycleOfNextCrtRead() override			{ return 1<<30; }	// ZXSP++ only

	int32	cpuCycleOfFrameFlyback() override			{ return tv_decoder.getMaxCyclesPerFrame(); }
	int32	updateScreenUpToCycle(int32 cc) override	{ tv_decoder.updateScreenUpToCycle(cc); return 1<<30; } // ZXSP only
	void	crtcRead(int32 cc, uint byte) override;
	uint8	interruptAtCycle(int32, uint16) override;

//	int32	getCurrentFramebufferIndex() override		{ return fbu_idx; }
//	int32	framebufferIndexForCycle(int32 cc) override	{ return fbu_idx_for_cc(cc); }

	void	set60Hz(bool=1) override;
	int32	getCcPerFrame() volatile const override		{ return cc_per_frame; }
	void	setupTiming() override {}

	void	enableMicOut(bool f);

private:
	TVDecoderMono tv_decoder;

	int32 cc_per_frame = 0;
	uint8 current_lcntr;	// 3 bit low line counter [0..7] of ula
	bool  vsync_on;

	void mic_out(Time, Sample);
	void reset();
};














