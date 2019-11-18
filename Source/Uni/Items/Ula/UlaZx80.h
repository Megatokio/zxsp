#pragma once
/*	Copyright  (c)	Günter Woigk 2008 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "UlaMono.h"


class UlaZx80 : public UlaMono
{
public:
	explicit UlaZx80(Machine*);
	virtual ~UlaZx80();

// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	int32	doFrameFlyback(int32 cc) override;
	void	drawVideoBeamIndicator(int32 cc) override;

	void	setBorderColor(uint8) override { border_color = 0xFF; /* White. ZX80 has no "border color" */ }

	int32	cpuCycleOfInterrupt() override			{ return 0; }
	int32	cpuCycleOfIrptEnd() override			{ return 0; }
	int32	cpuCycleOfFrameFlyback() override		{ return cc_for_frame_end_max(); }

	int32	cpuCycleOfNextCrtRead() override		{ return 1<<30; }
	int32	updateScreenUpToCycle(int32 cc) override;
	void	crtcRead(int32 cc, uint byte) override;
	uint8	interruptAtCycle(int32, uint16) override;

	int32	getCurrentFramebufferIndex() override		{ return fbu_idx; }
	int32	framebufferIndexForCycle(int32 cc) override	{ return fbu_idx_for_cc(cc); }

	void	set60Hz(bool=1) override;
	int32	getCcPerFrame() volatile const override		{ return cc_per_frame; }
	void	setupTiming() override {}

	void	enableMicOut(bool f);


// ====================== PRIVATE ===========================================================

private:
	void	sync_on					( int32 );
	void	sync_off				( int32 );
	void	self_trigger_hsync		( int32 );
	void	pad_video_bytes			( int32, uchar );
	void	pad_video_bytes			( int32 );
	void	mic_out					( Time, Sample );
	void	reset					();
	void	send_frame_to_screen	(int32 cc);

// limits:
	static const int
		bytes_per_row			= 52,					// 207/4
		bytes_per_row_min		= bytes_per_row - 4,
		bytes_per_row_max		= bytes_per_row + 4,	// Min( bytes_per_row+4, UlaMono::max_bytes_per_row )

		bytes_per_frame_50		= 3250000/50/4,
		bytes_per_frame_50_min	= bytes_per_frame_50 - bytes_per_row * 8/*rows*/,
		bytes_per_frame_50_max	= bytes_per_frame_50 + bytes_per_row * 8/*rows*/,

		bytes_per_frame_60		= 3250000/60/4,
		bytes_per_frame_60_min	= bytes_per_frame_60 - bytes_per_row * 8/*rows*/,
		bytes_per_frame_60_max	= bytes_per_frame_60 + bytes_per_row * 8/*rows*/,

		bytes_per_frame_min		= bytes_per_frame_60_min,
		bytes_per_frame_max		= bytes_per_frame_50_max,

		cc_per_row				= 4* bytes_per_row,
		cc_per_row_min			= 4* bytes_per_row_min,
		cc_per_row_max			= 4* bytes_per_row_max,

		cc_per_frame_50			= 4* bytes_per_frame_50,
		cc_per_frame_50_min		= 4* bytes_per_frame_50_min,
		cc_per_frame_50_max		= 4* bytes_per_frame_50_max,

		cc_per_frame_60			= 4* bytes_per_frame_60,
		cc_per_frame_60_min		= 4* bytes_per_frame_60_min,
		cc_per_frame_60_max		= 4* bytes_per_frame_60_max,

		cc_per_frame_min		= 4* bytes_per_frame_min,
		cc_per_frame_max		= 4* bytes_per_frame_max,

		cc_for_vsync_min		= cc_per_row*4;

// ula state:
	static const int cc_frame_start = 0;	// cc of current frame start
	int32	cc_for_frame_end_min()	{ return cc_frame_start + (is60hz ? cc_per_frame_60_min : cc_per_frame_50_min); }
											// cc when vsync starts working
	int32	cc_for_frame_end_max()	{ return cc_frame_start + (is60hz ? cc_per_frame_60_max : cc_per_frame_50_max); }
											// cc when vsync will self-trigger
	int32	cc_for_row_end_min()	{ return cc_row_start+cc_per_row_min; }		// cc when hsync starts working
	int32	cc_for_row_end_max()	{ return cc_row_start+cc_per_row_max; }		// cc when hsync will self-trigger
	bool	sync_on()				{ return cc_sync_on>=cc_sync_off; }			// get sync state
	bool	sync_off()				{ return cc_sync_on<cc_sync_off; }
	int		fbu_idx_of_row_start()	{ return current_row * bytes_per_row; }
	int		fbu_idx_for_cc(int32 cc){ return fbu_idx_of_row_start() + (cc-cc_row_start)/4; }

	//bool	mic_out_enabled;// MIC_OUT setting
	int32	cc_frame_end;	// cc of current frame end (after ffb successfully triggered, else 0)
	int32	cc_row_start;	// cc of current row start
	int32	cc_sync_on;		// cc of last sync on switching
	int32	cc_sync_off;	// cc of last sync off switching
	int		current_row;	// crt raster row
	int		current_lcntr;	// 3 bit low line counter [0..7] of ula
	int		fbu_idx;		// current pixel byte deposition index in frame_buffer[] dep. on cc_sync_on or cc_sync_off
	int32	cc_per_frame;
};











