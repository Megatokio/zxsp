// Copyright (c) 2008 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Ula.h"


namespace zxsp
{

class UlaJupiter : public Ula
{
public:
	enum AudioMode { mic_out_only, speaker_only, mixed_audio };

private:
	AudioMode audio_mode;

public:
	UlaJupiter(Machine*, bool is60hz);

	void	  setAudioMode(AudioMode m) { audio_mode = m; }
	AudioMode getAudioMode() { return audio_mode; }

protected:
	~UlaJupiter() override;

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	//void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	// Crtc interface:
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	int32 updateScreenUpToCycle(int32 cc) override;
	void  markVideoRam();
	void  setBorderColor(uint8) override {} // can't be set
	void  set60Hz(bool = 1) override;

	// Ula interface:
	int32 cpuCycleOfInterrupt() override { return 0; }
	int32 cpuCycleOfIrptEnd() override { return 8 * cc_per_line; }
	int32 cpuCycleOfFrameFlyback() override { return lines_per_frame * cc_per_line; }
	void  setupTiming() override {}
};

} // namespace zxsp
