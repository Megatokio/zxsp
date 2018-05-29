/*	Copyright  (c)	Günter Woigk 2008 - 2018
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef ULAZX81_H
#define ULAZX81_H


#include "UlaMono.h"


class UlaZx81 : public UlaMono
{
public:
	explicit UlaZx81(Machine*);
	virtual ~UlaZx81();

// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	int32	doFrameFlyback			(int32 cc) override;
	void	drawVideoBeamIndicator	(int32 cc) override;

	void	setBorderColor			(uint8) override { border_color = 0xFF; /* White. ZX80 has no "border color" */ }

	int32	cpuCycleOfInterrupt		() override				{ return 0; }
	int32	cpuCycleOfIrptEnd		() override				{ return 0; }
	int32	cpuCycleOfFrameFlyback	() override				{ return cc_per_frame_max; }

	int32	cpuCycleOfNextCrtRead() override				{ return 1<<30; }
	int32	updateScreenUpToCycle	(int32 cc) override;
	void	crtcRead				(int32 cc, uint byte) override;
	int32	nmiAtCycle				(int32 cc_nmi);
	uint8	interruptAtCycle		(int32, uint16) override;

	int32	getCurrentFramebufferIndex() override			{ return tv_idx; }
	int32	framebufferIndexForCycle(int32 cc) override		{ return tv_idx_for_cc(cc); }

	void	set60Hz					(bool=1) override;
	int32	getCcPerFrame			() volatile const override	{ return cc_per_frame; }
	void	setupTiming() override {}

	void	enableMicOut			(bool f);


// ====================== PRIVATE ===========================================================

private:

	//bool	mic_out_enabled;		// MIC_OUT setting

	bool	tv_sync_state;			// stores current sync state. req. for tv_update()
	int32	tv_cc_sync;				// cc of last sync state change
	int32	tv_cc_row_start;		// cc when current row stated: sync_off or self-triggered
	int32	tv_cc_frame_start;		// cc when frame started: 0 for this frame or cc of next frame after ffb
	int32	tv_idx;					// fbu[idx] for next byte
	int32	tv_row;					// current row

	int		ula_lcntr;				// 3 bit low line counter [0..7] of ula
	bool	ula_sync_state;			// sync as set by in(FE) / out(FF)
	bool	ula_nmi_enabled;		// set by out(FD) / out(FE)
	int32	ula_cc_next_nmi;

	int32	cc_per_frame;

	void	reset					();
	void	mic_out					( Time, Sample );

// limits:
	static const int
		bytes_per_row			= 52,							// in frame_buffer[]
		bytes_per_row_min		= bytes_per_row - 4,
		bytes_per_row_max		= bytes_per_row + 4,			// Min( bytes_per_row+4, UlaMono::max_bytes_per_row )

		bytes_per_frame_50		= 3250000/50/4,
		bytes_per_frame_50_min	= bytes_per_frame_50 - 8 * bytes_per_row,	// ±8 rows
		bytes_per_frame_50_max	= bytes_per_frame_50 + 8 * bytes_per_row,	// ±8 rows

		bytes_per_frame_60		= 3250000/60/4,
		bytes_per_frame_60_min	= bytes_per_frame_60 - 8 * bytes_per_row,	// ±8 rows
		bytes_per_frame_60_max	= bytes_per_frame_60 + 8 * bytes_per_row,	// ±8 rows

		bytes_per_frame_min		= bytes_per_frame_60_min,
		bytes_per_frame_max		= bytes_per_frame_50_max,

		cc_per_row				= 207,							// ula: nmi and hsync generator
		cc_per_row_min			= 4* bytes_per_row_min,			// tv: hsync
		cc_per_row_max			= 4* bytes_per_row_max,			// tv: self-hsync

		cc_per_frame_50			= 4* bytes_per_frame_50,
		cc_per_frame_50_min		= 4* bytes_per_frame_50_min,
		cc_per_frame_50_max		= 4* bytes_per_frame_50_max,

		cc_per_frame_60			= 4* bytes_per_frame_60,
		cc_per_frame_60_min		= 4* bytes_per_frame_60_min,
		cc_per_frame_60_max		= 4* bytes_per_frame_60_max,

		cc_per_frame_min		= 4* bytes_per_frame_min,		// tv: vsync
		cc_per_frame_max		= 4* bytes_per_frame_max,		// tv: self-vsync

		cc_for_vsync_min		= cc_per_row * 4;				// tv: vsync: 4 rows min.

// virtual TV set:

// antenna cable interface:
	void	tv_sync_on				( int32 cc );	// activate sync.	assumes: sync was off
	void	tv_sync_off				( int32 cc );	// deactivate sync.	assumes: sync was on
	void	tv_update				( int32 cc );	// update frame buffer up to cycle. self-trigger sync as needed.
	void	tv_store_byte			( uint8 );		// store one pixel byte now and increment time.
// internal:
	void	tv_pad_pixels			( int32, uint8 );
	void	tv_self_trigger			( int32 );
	void	tv_do_frame_flyback		( );

	int32	cc_frame_end_min()		{ return tv_cc_frame_start + cc_per_frame_min; } // cc when vsync starts working
	int32	cc_frame_end_max()		{ return tv_cc_frame_start + cc_per_frame_max; } // cc when vsync will self-trigger

	int32	cc_row_end_min()		{ return tv_cc_row_start + cc_per_row_min; } // cc when hsync starts working
	int32	cc_row_end_max()		{ return tv_cc_row_start + cc_per_row_max; } // cc when hsync will self-trigger

	int		tv_idx_of_row_start()	{ return tv_row * bytes_per_row; }
	int		tv_idx_for_cc(int32 cc)	{ return tv_idx_of_row_start() + (cc-tv_cc_row_start)/4; }

// ULA:
	void	ula_run_counters		( int32 );

};



#endif









