#pragma once
// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IoInfo.h"
#include "Memory.h"
#include "Ula.h"


class UlaZxsp : public Ula
{
	friend class SpectraVideo;

protected:
	int32				  cc_per_side_border;  // Zeit für Seitenborder+Strahlrücklauf
	int32				  cc_waitmap_start;	   // Ab wann die Waitmap benutzt werden muss
	int32				  cc_screen_start;	   // Erster cc für einen CRT Backcall
	int32				  cc_waitmap_end;	   // Ab wann nicht mehr
	int32				  cc_frame_end;		   // Total cpu clocks per Frame
	static constexpr uint bytes_per_octet = 2; // bytes needed to store 8 pixels

	uint  waitmap_size; // cc
	uint8 waitmap[256]; // up to (16+32+16)*4 cc

	Z80*	  cpu;
	MemoryPtr ram;

	// CRTC:
	int32  current_frame; // counter, used for flash phase
	int32  ccx;			  // next cc for reading from video ram
	uint8* attr_pixel;	  // specci screen attribute and pixel tupels
	// IoInfo*	ioinfo;
	// uint		ioinfo_count;
	// uint		ioinfo_size;
	uint8*	alt_attr_pixel; // alternate data set
	IoInfo* alt_ioinfo;
	uint	alt_ioinfo_size;

	Sample earin_threshold_mic_lo;
	Sample earin_threshold_mic_hi;


protected:
	UlaZxsp(Machine*, isa_id, cstr oaddr, cstr iaddr);
	~UlaZxsp() override;
	void setupTiming() override;


	// ---- PUBLIC ------------------------------------------------------

public:
	static const int MIN_LINES_BEFORE_SCREEN = 24,
					 MAX_LINES_BEFORE_SCREEN = 80, // nominal: 63/64, Pentagon: 80
		MIN_LINES_AFTER_SCREEN				 = 24,
					 MAX_LINES_AFTER_SCREEN	 = 2000, // note: used for padding for cpu clock overdrive!
		MIN_BYTES_PER_LINE					 = 4 + 32 + 4,
					 MAX_BYTES_PER_LINE		 = 256 / 4; // sizeof(waitmap)/cc_per_byte

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
	bool	getFlashPhase() { return (current_frame >> 4) & 1; }
	int32	cpuCycleOfNextCrtRead() override { return ccx; }
	uint8*	newAttrPixelArray() { return new uint8[32 * 24 * 8 * bytes_per_octet]; }
};


class UlaTk90x : public UlaZxsp
{
public:
	UlaTk90x(Machine*, bool is60hz);
	void set60Hz(bool = 1) override;
};
