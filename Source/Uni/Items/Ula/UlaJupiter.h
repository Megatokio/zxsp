#pragma once
// Copyright (c) 2008 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "UlaMono.h"


class UlaJupiter : public UlaMono
{
public:
	enum	AudioMode	{ mic_out_only, speaker_only, mixed_audio };

private:
	AudioMode audio_mode;

public:
	UlaJupiter(Machine*, uint fps);
	~UlaJupiter();

	void	setAudioMode	(AudioMode m)		{ audio_mode = m; }
	AudioMode getAudioMode	()					{ return audio_mode; }

protected:
// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

// Crtc interface:
	int32	doFrameFlyback(int32 cc) override;
	void	drawVideoBeamIndicator(int32 cc) override;
	void	markVideoRam() override;
	int32	cpuCycleOfNextCrtRead() override	{ return 1<<30; }
	int32	updateScreenUpToCycle(int32 cc) override;
	void	setBorderColor(uint8) override		{}	// can't be set
	void	set60Hz(bool=1) override;

// Ula interface:
	int32	cpuCycleOfInterrupt() override		{ return 0; }
	int32	cpuCycleOfIrptEnd() override		{ return 8 * cc_per_line; }
	int32	cpuCycleOfFrameFlyback() override	{ return lines_per_frame * cc_per_line; }
	void	setupTiming() override				{}

// UlaMono:
	int32	getCurrentFramebufferIndex() override		{ return 0; /*z.zt. nur full frames, kein highres update*/ }
	int32	framebufferIndexForCycle(int32 cc) override	{ return cc/4; }
};












