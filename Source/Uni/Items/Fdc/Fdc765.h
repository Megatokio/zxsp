/*	Copyright  (c)	Günter Woigk 2012 - 2019
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

#ifndef FDC765_H
#define FDC765_H

#include "Fdc.h"
#include "FloppyDiskDrive.h"


//	Emulation of a NEC µPD765 (with some errors…)
//	which is connected like this:
//
//	no DMA			send_byte_to_dma() and read_byte_from_dma() do nothing but may be reimplemented
//	no interrupts	raise_interrupt() and clear_interrupt() just set this.interrupt but may be reimplemented
//	no FM			MFM is always assumed.


class Fdc765 : public Fdc
{
	uint	sm_state;					// for coroutine macros
	uint	sm_stack[8];
	uint	sm_sp;						// SM stack pointer

	// FDC command and arguments:
	uint32	CMD;						// stored as mask: CMD = 1<<CMD !!
	uint8	head_unit,track_id,head_id,sector_id,log2sectorsize,gap3len;
	uint8	last_sector_id,num_sectors, fillbyte,sectorsize,step;
	uint8	requested_track[4];			// target track in step command

	// extracted data:
	bool	multitrack,mfm,skip_bit;	// bits 5…7 from CMD
	uint	unit;						// selected drive: from HU (HeadUnit)
	uint	head;						// selected head:  from HU (HeadUnit)

	// FDC status registers:
	uint8	MSR,SR0,SR1,SR2;			// main SR, result phase SR0 … 2

	// Drive settings from command SPECIFY:
	Time	headloadtime;
	Time	headunloadtime;
	Time	steprate;
	bool	dma_disabled;

	// current track: (FDC internal counters)
	uint	track[4];					// current tracks (as believed by internal counters)

	// internal timers:
	Time	when_head_unloaded;			// internal timer: head unload time
	Time	timeout;					// step, head load, or i/o timeout

	// FDC state
	bool	terminal_count;
	bool	ready;						// last seen drive state (for interrupt)

	// misc. state machine variables:
	uint	idams, bytepos, n, z, size, ssize, indexpulses;
	uint16	crc;
	uint8	byte, actual_log2ssize; // byte_for_cpuxxxxx, byte_from_cpuxxxxx;
	bool	f,eq,le,ge,crc_on,skip;

	// up to which time the state machine ran:
	Time	time;

public:
	Fdc765	(Machine*, isa_id, Internal internal, cstr o_addr, cstr i_addr);

	uint8	readMainStatusRegister(Time);
	uint8	readDataRegister(Time);
	void	writeDataRegister(Time,uint8);

	void	initForSnapshot(int32 cc) override;

protected:
	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time, int32 cc) override;
	void	input			(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
	void	output			(Time, int32 cc, uint16 addr, uint8 byte) override				= 0;
	void	audioBufferEnd	(Time) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD& fd) const throws override;
	void	loadFromFile	(FD& fd) throws override;

private:
	// DOIT:
	void	run_statemachine(Time);
	void	start_crc()			{ crc_on = yes; crc = 0xffff; }
	void	update_crc();
	void	cancel_crc()		{ crc_on = no; }
	bool	scan_satisfied();
	uint8	SR3();

VIR	bool	is_fault()			{ return drive->is_error; }
VIR	bool	is_2sided()			{ return drive->is_2sided; }
	bool	is_wprot()			{ return drive->is_wprot; }
	bool	is_track0()			{ return drive->is_track0; }
	bool	is_ready()			{ return drive->is_ready; }
	bool	is_atIndex()		{ return drive->is_atindex; }
	void	_init();
};


#endif





















