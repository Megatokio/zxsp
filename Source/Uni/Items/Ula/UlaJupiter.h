#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ula.h"


class UlaJupiter : public Ula
{
public:
	enum AudioMode { mic_out_only, speaker_only, mixed_audio };

private:
	AudioMode			 audio_mode;
	static constexpr int frame_w			 = 32 + 8 + 4 + 8; // frame width [bytes] == address offset per scan line
	static constexpr int screen_w			 = 32;			   // screen width [bytes]
	static constexpr int screen_x0			 = 0;			   // hor. screen offset inside frame scan lines [bytes]
	static constexpr int max_lines_per_frame = 312;			   // for frame buffers

	uint8* frame_data;	// frame buffer for decoded monochrome video signal
	uint8* frame_data2; // frame buffer for decoded monochrome video signal

public:
	UlaJupiter(Machine*, uint fps);
	~UlaJupiter() override;

	void	  setAudioMode(AudioMode m) { audio_mode = m; }
	AudioMode getAudioMode() { return audio_mode; }

protected:
	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	void reset(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void	audioBufferEnd	(Time t) override;
	// void	videoFrameEnd	(int32 cc) override;

	// Crtc interface:
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	void  markVideoRam() override;
	int32 cpuCycleOfNextCrtRead() override { return 1 << 30; }
	int32 updateScreenUpToCycle(int32 cc) override;
	void  setBorderColor(uint8) override {} // can't be set
	void  set60Hz(bool = 1) override;

	// Ula interface:
	int32 cpuCycleOfInterrupt() override { return 0; }
	int32 cpuCycleOfIrptEnd() override { return 8 * cc_per_line; }
	int32 cpuCycleOfFrameFlyback() override { return lines_per_frame * cc_per_line; }
	void  setupTiming() override {}
};
