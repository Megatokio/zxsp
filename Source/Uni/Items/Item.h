#pragma once
/*	Copyright  (c)	Günter Woigk 2004 - 2019
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

#include <QObject>
#include "zxsp_types.h"
#include "IsaObject.h"
#include "IoInfo.h"
#include "kio/peekpoke.h"
#define VIR virtual
#define EXT extern

extern uint16	bitsForSpec	( cstr s );
extern uint16	maskForSpec	( cstr s );
extern cstr		And			( cstr a, cstr b );

enum Internal
{
	internal = 1,
	external = 0
};


class Item : public IsaObject
{
	Item(const Item&) = delete;
	Item& operator=(const Item&) = delete;

protected:
	Machine		*machine;
	Item		*_next, *_prev;
	uint16		in_mask;
	uint16		in_bits;
	uint16		out_mask;
	uint16		out_bits;
	Internal    _internal;

	IoInfo*		ioinfo;
	uint		ioinfo_count;
	uint		ioinfo_size;

protected:
	bool		ramdis_in;		// RAMCS state    ZX80/81
	bool		romdis_in;		// ROMCS state    ZX81/ZXSP/128/+2/+2A/+3

protected:
	Item(Machine*, isa_id, isa_id grp, Internal, cstr o_addr, cstr i_addr);

	void	grow_ioinfo		();
	void	record_ioinfo	(int32 cc, uint16 addr, uint8 byte, uint8 mask=0xff);

	bool	event			(QEvent*e);


// ---------------- P U B L I C -------------------

public:
	virtual ~Item();

	Item*	prev			() const		{ return _prev; }
	Item*	next			() const		{ return _next; }
	Machine*getMachine		() const		{ return machine; }
	void	linkBehind		(Item*);
	void	unlink			();
	bool	matchesIn		(uint16 addr)	{ return (addr & in_mask)  == in_bits;  }
	bool	matchesOut		(uint16 addr)	{ return (addr & out_mask) == out_bits; }
	bool    isInternal      ()				{ return _internal; }
	bool    isExternal      ()				{ return !_internal; }

	bool	is_locked		() volatile const;
	void	lock			() volatile const;
	void	unlock			() volatile const;

// Item interface:
VIR void	powerOn			(/*t=0*/ int32 cc);
VIR void	reset			(Time t, int32 cc);
VIR void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask);
VIR void	output			(Time t, int32 cc, uint16 addr, uint8 byte);
VIR uint8	handleRomPatch	(uint16 pc, uint8 o);							// returns new opcode
VIR	uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte);	// for memory mapped i/o
VIR	void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte);	// for memory mapped i/o
VIR void	audioBufferEnd	(Time t);
VIR void	videoFrameEnd	(int32 cc);
VIR void	saveToFile		(FD&) const throws;
VIR void	loadFromFile	(FD&) throws;
VIR	void	triggerNmi		();

// Behandlung von daisy-chain bus-signalen
// Default: einfach durchleiten
//			dann werden auch ramdis und romdis nicht aktualisiert!
//			ein Item das romdis benutzt, muss logischerweise auch romCS() ersetzen.
VIR void    ramCS           (bool active);		// RAM_CS:  ZX80, ZX81
VIR void    romCS           (bool active);		// ROM_CS:  ZX81, ZXSP, ZX128, +2; ROMCS1+ROMCS2: +2A, +3
};


//===================================================================


// reset item
// sequence: cpu .. lastitem
// timing is as for input / output:
// 	 Time may overshoot dsp buffer but is limited to fit in dsp buffer + stitching
// 	 (this is only a concern if the machine is running veerryy slowwllyy)
// 	 cc may overshoot cc_per_frame a few cycles
inline void	Item::reset			(Time, int32){}
inline void	Item::input			(Time, int32, uint16, uint8&, uint8&){}
inline void	Item::output		(Time, int32, uint16, uint8){}
inline void	Item::audioBufferEnd(Time){}
inline void	Item::videoFrameEnd	(int32){}
inline void	Item::saveToFile	(FD&) const throws {}
inline void	Item::loadFromFile	(FD&) throws {}

// The generic NMI button in the main menubar was pressed:
// search for a NMI supproting item to handle it.
// Falls through to the CPU which _will_ handle it.
// Note: normally an NMI makes only sense if there is HW attached to handle it.
inline void Item::triggerNmi()	{ _prev->triggerNmi(); }

// memory r/w/x patches:
// methods must forward call to prev() if not hit
// chain ends in Mmu
inline uint8 Item::handleRomPatch(uint16 pc,uint8 o)				{ return _prev->handleRomPatch(pc,o); }
inline uint8 Item::readMemory(Time t, int32 cc, uint16 a, uint8 n)	{ return _prev->readMemory(t,cc,a,n); }
inline void Item::writeMemory(Time t, int32 cc, uint16 a, uint8 n)	{ _prev->writeMemory(t,cc,a,n); }

// RAM_CS daisy-chain:	ZX80, ZX81
// ROM_CS daisy-chain:	ZX81, ZXSP, ZX128, +2, +2A, +3
// chain ends in Mmu
inline void Item::ramCS(bool active)	{ _prev->ramCS(active); }
inline void Item::romCS(bool active)	{ _prev->romCS(active); }

inline void Item::record_ioinfo(int32 cc, uint16 addr, uint8 byte, uint8 mask)
{
	if(ioinfo_count==ioinfo_size) grow_ioinfo();
	ioinfo[ioinfo_count++] = IoInfo(cc,addr,byte,mask);
}





























































