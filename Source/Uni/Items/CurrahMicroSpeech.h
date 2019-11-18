/*	Copyright  (c)	Günter Woigk 2014 - 2019
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


#ifndef CURRAHMICROSPEECH_H
#define CURRAHMICROSPEECH_H

#include "Item.h"
#include "Memory.h"
class SP0256;


class CurrahMicroSpeech : public Item
{
	SP0256*		sp0256;
	MemoryPtr	rom;

	bool		enable_state;		// rom paged in and i/o ports enabled
	void		toggle_enable_state();

	// for display in Inspector:
	uint8		pitch;				// 0x00 or 0x40
	void		add_history(uint8);

	Frequency	clock, current_clock;

public:
	uint8	history[16];
	uint	lastrp,lastwp;
	uint	pause;					// after history[wp]


public:
	explicit CurrahMicroSpeech(Machine*);
	~CurrahMicroSpeech();

	void	setHifi(bool) volatile;		// main thread only
	bool	isHifi() const volatile;

// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) throws/*bad alloc*/ override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	uint8	handleRomPatch	(uint16,uint8) override;				// returns new opcode
	void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override; // memory mapped i/o
	void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override; // memory mapped i/o
};


#endif





















