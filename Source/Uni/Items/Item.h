#pragma once
// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IoInfo.h"
#include "IsaObject.h"
#include "kio/peekpoke.h"
#include "zxsp_types.h"

extern uint16 bitsForSpec(cstr s);
extern uint16 maskForSpec(cstr s);
extern cstr	  And(cstr a, cstr b);

enum Internal { internal = 1, external = 0 };


class Item : public IsaObject
{
	Item(const Item&)			 = delete;
	Item& operator=(const Item&) = delete;

protected:
	Machine* machine;
	Item*	 _next;
	Item*	 _prev;
	uint16	 in_mask;
	uint16	 in_bits;
	uint16	 out_mask;
	uint16	 out_bits;
	Internal _internal;

	IoInfo* ioinfo;
	uint	ioinfo_count;
	uint	ioinfo_size;

protected:
	bool ramdis_in; // RAMCS state    ZX80/81
	bool romdis_in; // ROMCS state    ZX81/ZXSP/128/+2/+2A/+3

protected:
	Item(Machine*, isa_id, isa_id grp, Internal, cstr o_addr, cstr i_addr);

	void grow_ioinfo();
	void record_ioinfo(int32 cc, uint16 addr, uint8 byte, uint8 mask = 0xff);


	// ---------------- P U B L I C -------------------

	friend class Machine;
	virtual ~Item() override;

public:
	Item*	 prev() const { return _prev; }
	Item*	 next() const { return _next; }
	Machine* getMachine() const { return machine; }
	void	 linkBehind(Item*);
	void	 unlink();
	bool	 matchesIn(uint16 addr) { return (addr & in_mask) == in_bits; }
	bool	 matchesOut(uint16 addr) { return (addr & out_mask) == out_bits; }
	bool	 isInternal() { return _internal; }
	bool	 isExternal() { return !_internal; }

	bool is_locked() const volatile;
	void lock() const volatile;
	void unlock() const volatile;

	// Item interface:
	virtual void  powerOn(/*t=0*/ int32 cc);
	virtual void  reset(Time t, int32 cc);
	virtual void  input(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask);
	virtual void  output(Time t, int32 cc, uint16 addr, uint8 byte);
	virtual uint8 handleRomPatch(uint16 pc, uint8 o);					  // returns new opcode
	virtual uint8 readMemory(Time t, int32 cc, uint16 addr, uint8 byte);  // for memory mapped i/o
	virtual void  writeMemory(Time t, int32 cc, uint16 addr, uint8 byte); // for memory mapped i/o
	virtual void  audioBufferEnd(Time t);
	virtual void  videoFrameEnd(int32 cc);
	virtual void  triggerNmi();

	// Handling of daisy chain bus signals
	// Default: just forward the signal
	//			in this case ramdis and romdis are not updated!
	//			an Item that uses romdis must override romCS().
	virtual void ramCS(bool active); // RAM_CS:  ZX80, ZX81
	virtual void romCS(bool active); // ROM_CS:  ZX81, ZXSP, ZX128, +2; ROMCS1+ROMCS2: +2A, +3
};


//===================================================================


// reset item
// sequence: cpu .. lastitem
// timing is as for input / output:
// 	 Time may overshoot dsp buffer but is limited to fit in dsp buffer + stitching
// 	 (this is only a concern if the machine is running veerryy slowwllyy)
// 	 cc may overshoot cc_per_frame a few cycles
inline void Item::reset(Time, int32) {}
inline void Item::input(Time, int32, uint16, uint8&, uint8&) {}
inline void Item::output(Time, int32, uint16, uint8) {}
inline void Item::audioBufferEnd(Time) {}
inline void Item::videoFrameEnd(int32) {}

// The generic NMI button in the main menubar was pressed:
// search for a NMI supproting item to handle it.
// Falls through to the CPU which _will_ handle it.
// Note: normally an NMI makes only sense if there is HW attached to handle it.
inline void Item::triggerNmi() { _prev->triggerNmi(); }

// memory r/w/x patches:
// methods must forward call to prev() if not hit
// chain ends in Mmu
inline uint8 Item::handleRomPatch(uint16 pc, uint8 o) { return _prev->handleRomPatch(pc, o); }
inline uint8 Item::readMemory(Time t, int32 cc, uint16 a, uint8 n) { return _prev->readMemory(t, cc, a, n); }
inline void	 Item::writeMemory(Time t, int32 cc, uint16 a, uint8 n) { _prev->writeMemory(t, cc, a, n); }

// RAM_CS daisy-chain:	ZX80, ZX81
// ROM_CS daisy-chain:	ZX81, ZXSP, ZX128, +2, +2A, +3
// chain ends in Mmu
inline void Item::ramCS(bool active) { _prev->ramCS(active); }
inline void Item::romCS(bool active) { _prev->romCS(active); }

inline void Item::record_ioinfo(int32 cc, uint16 addr, uint8 byte, uint8 mask)
{
	if (ioinfo_count == ioinfo_size) grow_ioinfo();
	ioinfo[ioinfo_count++] = IoInfo(cc, addr, byte, mask);
}
