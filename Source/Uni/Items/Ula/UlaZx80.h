// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Ula.h"
#include "TVDecoderMono.h"


class UlaZx80 : public Ula
{
public:
	explicit UlaZx80(Machine*);
	virtual ~UlaZx80() override;

// Item interface:
	void  powerOn(/*t=0*/ int32 cc) override;
	void  reset(Time t, int32 cc) override;
	void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void  output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void  videoFrameEnd(int32 cc) override;

	void  markVideoRam() override			{}
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;

	void  setBorderColor(uint8) override	{ border_color = 0xFF; } // White: ZX80 has no "border color"
	int32 cpuCycleOfInterrupt() override	{ return 1<<30; }	// ZX80 has no regular timer interrupt
	int32 cpuCycleOfIrptEnd() override		{ return 1<<30; }
	int32 cpuCycleOfNextCrtRead() override	{ return 1<<30; }	// ZXSP++ only

	int32 cpuCycleOfFrameFlyback() override;
	int32 updateScreenUpToCycle(int32 cc) override;
	virtual void crtcRead(int32 cc, uint byte);
	uint8 interruptAtCycle(int32, uint16) override;

	void  set60Hz(bool=1) override;
	int32 getCcPerFrame() volatile const override	{ return tv_decoder.getCcPerFrame(); }
	void  setupTiming() override {}

	void  enableMicOut(bool f);

protected:
	UlaZx80(Machine* m, isa_id id, cstr oaddr, cstr iaddr);
	void mic_out(Time, int32 cc, bool bit);
	bool mic_in(Time, int32 cc);

	TVDecoderMono tv_decoder;

	uint8 lcntr;		// 3 bit low line counter [0..7] of ula
	bool  vsync;
};














