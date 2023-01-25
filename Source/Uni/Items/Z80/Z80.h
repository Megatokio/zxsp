#pragma once
// Copyright (c) 1996 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Z80 cpu emulation version 2.2.4
*/

#define	Z80_H
#include "Z80options.h"
#include "Item.h"


// ----	memory pages ----

#define		CPU_PAGESIZE		(1<<CPU_PAGEBITS)
#define		CPU_PAGEMASK		(CPU_PAGESIZE-1)
#define		CPU_PAGES			(0x10000>>CPU_PAGEBITS)


#ifdef __BIG_ENDIAN__			// m68k, ppc
	struct TwoBytes  { uint8 hi,lo; };
	struct FourBytes { uint8 flags3,flags2,flags1,data; };
#elif defined __LITTLE_ENDIAN__	// i386, pdp
	struct TwoBytes  { uint8 lo,hi; };
	struct FourBytes { uint8 data,flags1,flags2,flags3; };
#else
#error "endian macro"
#endif


typedef uint32 CoreByte;


struct PgInfo
{
	CoreByte*	 core_r;			// flags in the high bytes, data in the low byte
	CoreByte*	 core_w;			// flags in the high bytes, data in the low byte
	CoreByte*	 core_w2;			// shadow write page. MAY BE nullptr!
	uint8 const* waitmap_r;
	uint8 const* waitmap_w;
	int			 waitmap_r_size;
	int			 waitmap_w_size;

	CoreByte&	both_r				(uint16 addr)		{ return core_r[addr]; }
	CoreByte&	both_w				(uint16 addr)		{ return core_w[addr]; }
	uint8&		data_r				(uint16 addr)		{ return ((FourBytes*)(core_r+addr))->data; }
	uint8&		data_w				(uint16 addr)		{ return ((FourBytes*)(core_w+addr))->data; }
	uint8&		data_w2				(uint16 addr)		{ return ((FourBytes*)(core_w2+addr))->data; }	// core_w2 may be null!
	CoreByte&	both_w2				(uint16 addr)		{ return core_w2[addr]; }						// core_w2 may be null!
};


// ----	z80 registers ----

union Z80Regs
{
	uint16	nn[16];
	struct	 { uint16 af,bc,de,hl, af2,bc2,de2,hl2, ix,iy,pc,sp, iff, ir; };
   #if defined(__BIG_ENDIAN__)		// m68k, ppc
	struct	 { uint8 a,f,b,c,d,e,h,l, a2,f2,b2,c2,d2,e2,h2,l2, xh,xl,yh,yl,pch,pcl,sph,spl, iff1,iff2, i,r, im,xxx; };
   #elif defined(__LITTLE_ENDIAN__)	// i386, pdp
	struct	 { uint8 f,a,c,b,e,d,l,h, f2,a2,c2,b2,e2,d2,l2,h2, xl,xh,yl,yh,pcl,pch,spl,sph, iff1,iff2, r,i, im,xxx; };
   #else
	#error "endian error"
   #endif
};



class Z80 : public Item
{
public:
	explicit Z80(Machine*);
	virtual ~Z80();

// Item interface:
	void	powerOn			(int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask);
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte);
	//void	audioBufferEnd	(Time t);
	void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) override;


// Run the Cpu:
	int		run				(int32 end_cc, int32 end_ic, uint32 options);

// Instruction counter:
// note: .rzx file support: counts when r is incremented: prefixed instructions are counted += 2
	int32		instrCount		() const			{ return instr_cnt; }
	int32&		instrCountRef	()					{ return instr_cnt; }
	void		setInstrCount	(int32 n)			{ instr_cnt  = n; }

// T cycle counter:
	int32		cpuCycle		() volatile const	{ return cpu_cycle; }
	int32&		cpuCycleRef		()					{ return cpu_cycle; }
	void		setCpuCycle		(int32 cc)			{ cpu_cycle  = cc; }

// Interrupt:
	int32		interruptStart	() volatile const	{ return cc_irpt_on; }
	int32		interruptEnd	() volatile const	{ return cc_irpt_off; }
	bool		interruptPending() volatile const	{ return cpu_cycle>=cc_irpt_on && cpu_cycle<cc_irpt_off; }

	void		setInterrupt	(int32 a, int32 e)	{ cc_irpt_on = a; cc_irpt_off = e; }
	void		setInterrupt_wLen(int32 a,int32 dur){ cc_irpt_on = a; cc_irpt_off = a+dur; }
	void		raiseInterrupt	(int32 dur)			{ cc_irpt_on = cpu_cycle; cc_irpt_off = cpu_cycle+dur; }
	void		raiseInterrupt	()					{ cc_irpt_on = cpu_cycle; cc_irpt_off = 0x7fffffff; }
	void		clearInterrupt	()					{ cc_irpt_on = 0x7fffffff; }
	void		shiftInterrupt	(int32 cc)			{ cc_irpt_on -= cc; cc_irpt_off -= cc; }

// NMI:
	int32		nmiCycle		() volatile const	{ return cc_nmi; }
	bool		nmiPending		() volatile const	{ return cpu_cycle >= cc_nmi; }

	void		setNmi			(int32 cc)			{ cc_nmi = cc; }
	void		triggerNmi		() override			{ cc_nmi = cpu_cycle; }
	void		clearNmi		()					{ cc_nmi = 0x7fffffff; }
	void		shiftNmi		(int32 cc)			{ cc_nmi -= cc; }

	void		setCrtc			(Crtc* item)		{ crtc = item; }

// Registers:
	Z80Regs&	getRegisters	()					{ return registers; }
	Z80Regs volatile const&	getRegisters() volatile const { return registers; }

// Attach memory:
	void		mapDummyRomPage	(uint16 addr, uint16 size, CoreByte* data, uint8 const* waitmap, int wm_size);
	void		mapDummyWomPage	(uint16 addr, uint16 size, CoreByte* data, uint8 const* waitmap, int wm_size);
	void		mapDummyWom2Page(uint16 addr, uint16 size, CoreByte* data);
	void		mapRam			(uint16 addr, uint16 size, CoreByte* data, uint8 const* waitmap, int wm_size);
	void		mapRom			(uint16 addr, uint16 size, CoreByte* data, uint8 const* waitmap, int wm_size);
	void		mapWom			(uint16 addr, uint16 size, CoreByte* data, uint8 const* waitmap, int wm_size);
	void		mapMem			(uint16 addr, uint16 size, CoreByte* rdata,CoreByte* wdata,const uint8* wm,int wm_size);
	void		unmapRom		(uint16 addr, uint16 size, uint8 const* waitmap=nullptr, int wm_size=0);
	void		unmapWom		(uint16 addr, uint16 size, uint8 const* waitmap=nullptr, int wm_size=0);
	void		unmapRam		(uint16 addr, uint16 size, uint8 const* waitmap=nullptr, int wm_size=0);
	void		unmapAllMemory	()					{ unmapRam(0,0); unmapWom2(0,0); }
	void		unmapMemory		(CoreByte *a, uint32 size);
	void		mapWom2			(uint16 addr, uint16 size, CoreByte* data);
	void		unmapWom2		(uint16 addr, uint16 size);

// Access memory:
	PgInfo&		getPage			(uint16 addr)			{ return page[addr>>CPU_PAGEBITS]; }

	CoreByte*	rdPtr			(uint16 addr)			{ return getPage(addr).core_r + addr; }
	CoreByte*	wrPtr			(uint16 addr)			{ return getPage(addr).core_w + addr; }
	CoreByte*	wrPtr2			(uint16 addr)			{ return getPage(addr).core_w2 + addr; } // core_w2 may be null!
	bool		hasWrPtr2		(uint16 addr)			{ return getPage(addr).core_w2; }

	uint8		peek			(uint16 addr) const		{ return const_cast<Z80*>(this)->getPage(addr).both_r(addr); }
	void		poke			(uint16 addr, uint8 c)	{ getPage(addr).data_w(addr) = c; }
	uint16		peek2			(uint16 addr);
	void		poke2			(uint16 addr, uint16 n);
	uint16		pop2			();
	void		push2			(uint16 n);

	void		copyBufferToRam	(uint8 const* q, uint16 z_addr, uint16 _cnt)	noexcept;
	void		copyRamToBuffer	(uint16 q_addr,  uint8* z,      uint16 _cnt)	noexcept;

	void		copyBufferToRam	(const CoreByte *q, uint16 z_addr, uint16 _cnt)	noexcept;
	void		copyRamToBuffer	(uint16 q_addr,   CoreByte *z,     uint16 _cnt)	noexcept;

	void		readRamFromFile	(FD&, uint16 z_addr, uint16 _cnt) throws;
	void		writeRamToFile	(FD&, uint16 q_addr, uint16 _cnt) throws;

// Debugger:
	void		setStackBreakpoint	(uint16 sp)			{ stack_breakpoint = sp; }
	uint16		break_addr;

// Disassembler:
	int			getOpcodeLegalState	(uint16 ip) const;		enum { legal, illegal, weird };
	int			getOpcodeLength		(uint16 ip) const;
	char*		getOpcodeMnemo		(uint16 ip) const;
	char*		disassemble			(uint16 &ip) const;


static inline void c2b(const CoreByte* q, uint8* z, uint n) { for(uint i=0; i<n; i++) z[i] = q[i]; }
static inline void b2c(const uint8* q, FourBytes*z, uint n) { for(uint i=0; i<n; i++) z[i].data = q[i]; }
static inline void b2c(const uint8* q, CoreByte* z, uint n) { for(uint i=0; i<n; i++) ((FourBytes*)z)[i].data = q[i]; }
static inline void c2c(const CoreByte* q, CoreByte* z, uint n) { memcpy(z,q,n*sizeof(CoreByte)); }


// ===================== PRIVATE / PROTECTED =================================

private:
	Z80(const Z80&) = delete;
	Z80& operator= (const Z80&) = delete;

public:
	CoreByte	noreadpage[CPU_PAGESIZE];
	CoreByte	nowritepage[CPU_PAGESIZE];

protected:
	char*		_xword				(uint8 n, uint16 &ip) const;		// Disassembler
	void		reset_registers		();

	Crtc*		crtc;				// video controller, updated when writing to video ram
	PgInfo		page[CPU_PAGES];	// attached memory
	Z80Regs		registers;			// z80 registers
	int32		cpu_cycle;			// cpu T cycle counter
	int32		instr_cnt;			// instruction counter
	int32		cc_irpt_on;			// interrupt signal start cycle
	int32		cc_irpt_off;		// interrupt signal end cycle
	int32		cc_nmi;				// cycle for nmi trigger
	uint16		stack_breakpoint;	//
};


















