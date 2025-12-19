// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "IoInfo.h"
#include "Memory.h"
#include "Ula.h"
#include "VideoData.h"


namespace zxsp
{

class UlaZxsp : public Ula
{
	friend class SpectraVideo;

protected:
	int32 cc_per_side_border; // Zeit für Seitenborder+Strahlrücklauf
	int32 cc_waitmap_start;	  // Ab wann die Waitmap benutzt werden muss
	int32 cc_screen_start;	  // Erster cc für einen CRT Backcall
	int32 cc_waitmap_end;	  // Ab wann nicht mehr
	int32 cc_frame_end;		  // Total cpu clocks per Frame

	uint  waitmap_size; // cc
	uint8 waitmap[256]; // up to (16+32+16)*4 cc

	Z80*	  cpu;
	MemoryPtr ram;

	// CRTC:
	ZxspVideoData*	bucket		  = nullptr; // the currently constructed video frame
	int32			ccx			  = 0;		 // next cc for reading from video ram
	int				frame_counter = 0;		 // counter, used for flash phase
	VideoData::What frame_type;

	Sample earin_threshold_mic_lo;
	Sample earin_threshold_mic_hi;


protected:
	UlaZxsp(Machine*, isa_id, cstr oaddr, cstr iaddr);
	~UlaZxsp() override;
	void setupTiming() override;
	void put_bucket(ZxspVideoData* bucket, int32 cc);
	void get_bucket();


	// ---- PUBLIC ------------------------------------------------------

public:
	static constexpr int //
		MIN_LINES_BEFORE_SCREEN = 24,
		MAX_LINES_BEFORE_SCREEN = 80,	// nominal: 63/64, Pentagon: 80
		MIN_LINES_AFTER_SCREEN	= 24,	//
		MAX_LINES_AFTER_SCREEN	= 2000, // note: used for padding for cpu clock overdrive!
		MIN_BYTES_PER_LINE		= 4 + 32 + 4,
		MAX_BYTES_PER_LINE		= 256 / 4; // sizeof(waitmap)/cc_per_byte

public:
	explicit UlaZxsp(Machine*);

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
	// void		reset				(Time t, int32 cc) override;
	void input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void output(Time t, int32 cc, uint16 addr, uint8 byte) override;
	// void		audioBufferEnd		(Time t) override;
	// void		videoFrameEnd		(int32 cc) override;


	// Ula interface:
	void setBorderColor(uint8 b) override
	{
		b &= 7;
		border_color = b;
		ula_out_byte = (ula_out_byte & ~7) | b;
	}
	int32 doFrameFlyback(int32 cc) override;
	void  drawVideoBeamIndicator(int32 cc) override;
	// void		set60Hz				(bool=1) override;
	int32 addWaitCycles(int32 cc, uint16 addr) const volatile override;
	uint8 getFloatingBusByte(int32 cc) override;
	void  markVideoRam() override;

	// UlaZxsp:
	int32 cpuCycleOfFrameFlyback() override { return cc_frame_end; }
	int32 cpuCycleOfInterrupt() override { return 0; }
	int32 cpuCycleOfIrptEnd() override { return 32; }
	int32 getCpuCyclesPerFrame() { return cc_frame_end; }
	int32 getOctetsPerFrame() { return cc_frame_end / cc_per_byte; }
	int32 getScreenStart() const volatile { return cc_screen_start; }
	int32 getWaitmapStart() const volatile { return cc_waitmap_start; }
	int32 getWaitmapEnd() { return cc_waitmap_end; }
	uint8 getEarOutState() const volatile { return ula_out_byte & 0x10; }
	uint8 getMicOutState() const volatile { return ula_out_byte & 0x08; }

	// CRTC:
	cuint8* getWaitmap() { return waitmap; }
	int		getWaitmapSize() { return int(waitmap_size); }
	bool	hasWaitmap() const volatile { return waitmap_size != 0; }
	int32	updateScreenUpToCycle(int32 cc) override;
	bool	getFlashPhase() { return (frame_counter >> 4) & 1; }
};


class UlaTk90x : public UlaZxsp
{
public:
	UlaTk90x(Machine*, bool is60hz);
	void set60Hz(bool = 1) override;
};

} // namespace zxsp
