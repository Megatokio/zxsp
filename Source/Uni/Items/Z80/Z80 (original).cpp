/*	Copyright  (c)	Günter Woigk 1996 - 2012
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


	Z80 Emulator	version 2.2.4
					initially based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	1995-03-28 kio	Started work on C version
	1995-04-01 kio	Finished, yee it works!
	1995-04-06 kio	Removed "final bug": im2 interrupt handling
	1995-06-12 kio	revised EI_WITH_DELAY handling
	1995-06-12 kio	all Info-Calls and profiling added
	1995-06-12 kio	exact r register emulation completed
	1995-06-13 kio	All opcodes covered, no unsupported opcodes!
	1995-11-01 kio	Paged memory supported
	1995-11-26 kio	Engine now completely hardware independent!
	1996-03-28 kio	Core2Core() works even if src and dest block overlap at both ends
	1996-03-28 kio	removed option to switch off latency of 'EI' instruction
	1996-05-02 kio	LOAD_CC after IN and OUT added to make extra wait states possible
	1998-12-24 kio	started port to linux, rework for c++
	2000-02-12 kio	removed bug in LDDR emulation
	2002-02-06 kio	started work on cocoa version
	2004-11-08 kio	cleaned up source. removed 'class cpu' overhead and disfunct stuff
	2004-11-11 kio	wait cycle management and crt video callback finished
	2005-01-16 kio	wait testing cpu cycles validated against the real machine
	2006-10-06 kio	removed the 'class Cpu' layer, as it contained a lot of Z80 stuff...
	2008-06-09 kio	reworked run loop and exit codes; added instruction counting exit condition
	2008-06-14 kio	copyRamToBuffer(), ReadRamFromFile(), saveToFile() and vice versa functions
	2008-06-22 kio	korr: im=0 after reset
	2009-05-28 kio	Qt support, #defines for some classes, UnmapMem(), new initialization model
*/

#define SAFE	   2
#define LOG		   1
#define Z80_SOURCE		// -> include other class header files or typedefs only
#include "Z80.h"		// major header file
#include "Z80macros.h"	// required and optional macros
#include "Z80opcodes.h" // opcode enumeration
#include "Z80options.h" // customizations, other includes and/or typedefs
INIT_MSG


// dummy read and write pages for unmapped pages
// note: it's a bad idea to map these directly from the appl with size > CPU_PAGESIZE
static uint16 noreadpage[CPU_PAGESIZE];
static uint16 nowritepage[CPU_PAGESIZE];


// conversion table:   A -> Z80-flags with S, Z, V=parity and C=0
static uint8 zlog_flags[256] = {
#define FLAGS0(A)                                                                                                      \
  (A & 0x80) + ((A == 0) << 6) +                                                                                       \
	  (((~A + (A >> 1) + (A >> 2) + (A >> 3) + (A >> 4) + (A >> 5) + (A >> 6) + (A >> 7)) & 1) << 2)
#define FLAGS2(A) FLAGS0(A), FLAGS0((A + 1)), FLAGS0((A + 2)), FLAGS0((A + 3))
#define FLAGS4(A) FLAGS2(A), FLAGS2((A + 4)), FLAGS2((A + 8)), FLAGS2((A + 12))
#define FLAGS6(A) FLAGS4(A), FLAGS4((A + 16)), FLAGS4((A + 32)), FLAGS4((A + 48))
	FLAGS6(0), FLAGS6(64), FLAGS6(128), FLAGS6(192)};


// ==================================================================================


Z80::Z80(Z80_MACHINE* m, cstr name) : Z80_BASECLASS(m, isa_Z80, name)
{
	Z80_INFO_CTOR;
	ASSERT(sizeof(twobytes) == 2); // this should be a nop

	crtc		= NULL; // cathode ray tube controller
	cc_irpt_on	= 0;	// T cycle of next/regular interrupt ON ... OFF
	cc_irpt_off = 0;
	instr_cnt	= 0; // current instruction counter
	cpu_cycle	= 0;

	// init global dummy read page for non-mapped ram:	always reads 0xFF
	// note: no need to init nowritepage[]: 0x0000 is ok
	// note: no flags are set: dep. on implementation of PEEK and POKE these never add waitstates etc.
	for (int i = 0; i < CPU_PAGESIZE; i++) noreadpage[i] = 0x00FF; // flags=0x00, data=0xFF

	// init all pages with "no memory" and "no waitmap"
	unmapAllMemory();
}


Z80::~Z80() { Z80_INFO_DTOR; }


void Z80::init(int32 cc)
{
	Z80_BASECLASS::init(cc);
	crtc = machine->getCrtc();
	XXASSERT(crtc != NULL);
	instr_cnt = 0; // current instruction counter
	cpu_cycle = cc;
	reset_registers();
}


void Z80::reset(Time t, int32 cc)
{
	Z80_BASECLASS::reset(t, cc);
	cpu_cycle = cc;
	reset_registers();
}


void Z80::reset_registers()
{
	// reset registers:
	BC = DE = HL = IX = IY = PC = SP = BC2 = DE2 = HL2 = 0;
	RA = RA2 = RF = RF2 = RR = RI = 0;

	// reset other internal state:
	IFF1 = IFF2	 = disabled;   // disable interrupts
	registers.im = 0;		   // interrupt mode := 0			korr. kio 2008-06-22
	cc_nmi		 = 0x7FFFFFFF; // clear nmi pending FF
}


/*	Save to disk:
	save cpu internal state for later Init()
	does not save: memory page mapping, waitmaps, options and contents!
*/
void Z80::saveToFile(int fd) const throw(file_error, bad_alloc)
{
	Z80_BASECLASS::saveToFile(fd);
	write_data(fd, &registers);
	write_data(fd, &cpu_cycle);
	write_data(fd, &instr_cnt);
	write_data(fd, &cc_irpt_on);
	write_data(fd, &cc_irpt_off);
	write_data(fd, &cc_nmi);
	write_data(fd, &stack_breakpoint);
}

void Z80::loadFromFile(int fd) throw(file_error, bad_alloc)
{
	Z80_BASECLASS::loadFromFile(fd);
	read_data(fd, &registers);
	read_data(fd, &cpu_cycle);
	read_data(fd, &instr_cnt);
	read_data(fd, &cc_irpt_on);
	read_data(fd, &cc_irpt_off);
	read_data(fd, &cc_nmi);
	read_data(fd, &stack_breakpoint);
}


/*	shift cpu T cycle based by cc
	because video frame completed
*/
void Z80::videoFrameEnd(int32 cc)
{
	cpu_cycle -= cc;

	if (cc_irpt_on > 0) // test for fixed cc interrupt ((there should be a flag))
	{
		cc_irpt_on -= cc;
		cc_irpt_off -= cc;
	}

	if (cc_nmi != 0x7fffffff) // test for nmi
	{
		cc_nmi -= cc;
	}
}


// ======================================================================


uint16 Z80::peek2(uint16 addr) { return uint16(peek(addr)) + uint16(uint16(peek(addr + 1)) << 8); }

void Z80::poke2(uint16 addr, uint16 n)
{
	poke(addr, uint8(n));
	poke(addr + 1, uint8(n >> 8));
}

uint16 Z80::pop2()
{
	registers.sp += 2;
	return peek2(registers.sp - 2);
}

void Z80::push2(uint16 n)
{
	registers.sp -= 2;
	poke2(registers.sp, n);
}


inline void c2b(const uint16* q, uint8* z, uint n)
{
	for (uint i = 0; i < n; i++) z[i] = q[i];
}
inline void b2c(const uint8* q, twobytes* z, uint n)
{
	for (uint i = 0; i < n; i++) z[i].lo = q[i];
}


void Z80::copyRamToBuffer(uint16 q, uint8* z, uint16 cnt) throw()
{
	uint16 n = CPU_PAGESIZE - (q & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2b(rdPtr(q), z, n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::copyBufferToRam(const uint8* q, uint16 z, uint16 cnt) throw()
{
	uint16 n = CPU_PAGESIZE - (z & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		b2c(q, (twobytes*)wrPtr(z), n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::readRamFromFile(int fd, uint16 z, uint16 cnt) throw(file_error)
{
	uint8  bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE - (z & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		read_bytes(fd, bu, n);
		b2c(bu, (twobytes*)wrPtr(z), n);
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::writeRamToFile(int fd, uint16 q, uint16 cnt) throw(file_error)
{
	uint8  bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE - (q & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2b(rdPtr(q), bu, n);
		write_bytes(fd, bu, n);
		q += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

/* ----	attach ram/rom to address space --------------
		Attach real memory to cpu address space

		For best performance set CPU_PAGESIZE to the page size of the emulated machine.
		If you emulate multiple machines, set it to the lowest page size of any machine.

		addr	= cpu start address of page
		size	= size of page
					0x0000 is assumed to mean 0x10000 (entire address range)
					addr and size must be multiples of CPU_PAGESIZE
					addr+size must not extend beyond address space end 0x10000

		r_data,
		w_data	= pointer to page data

		waitmap	= wait cycles map for this page
		wmsize	= size of waitmap
					NULL  => no waitstates map
					access to waitmap: cc += waitmap[cc%wmsize]

		data and flags pointers in PgInfo struct point to the virtual location of address $0000
		so that you can access data using page.data_r[addr>>CPU_PAGEBITS][addr]
*/
void Z80::mapMem(uint16 addr, uint16 size, uint16* r_data, uint16* w_data, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(!r_data);
	XXTRAP(!w_data);
	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfass wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end
	XXTRAP(!waitmap && r_data[0] & cpu_waitmap);
	XXTRAP(!waitmap && w_data[0] & cpu_waitmap);

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);

	r_data -= addr;
	w_data -= addr;

	for (; a <= e; a++)
	{
		a->core_r	 = r_data;
		a->core_w	 = w_data;
		a->waitmap_w = a->waitmap_r = waitmap;
		a->waitmap_w_size = a->waitmap_r_size = wm_size;
	}
}

void Z80::mapRam(uint16 addr, uint16 size, uint16* data, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(!data);
	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfalls wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end
	XXTRAP(
		!waitmap &&
		data[0] & cpu_waitmap); // note: wartezyklen werden nur addiert, wo auch data[x]&cpu_waitmap gesetzt ist!

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++)
	{
		a->core_w = a->core_r = data;
		a->waitmap_w = a->waitmap_r = waitmap;
		a->waitmap_w_size = a->waitmap_r_size = wm_size;
	}
}

void Z80::mapWom(uint16 addr, uint16 size, uint16* data, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(!data);
	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfass wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end
	XXTRAP(!waitmap && data[0] & cpu_waitmap);

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++)
	{
		a->core_w		  = data;
		a->waitmap_w	  = waitmap;
		a->waitmap_w_size = wm_size;
	}
}

void Z80::mapRom(uint16 addr, uint16 size, uint16* data, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(!data);
	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfalls wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end
	XXTRAP(!waitmap && data[0] & cpu_waitmap);

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++)
	{
		a->core_r		  = data;
		a->waitmap_r	  = waitmap;
		a->waitmap_r_size = wm_size;
	}
}

/*	unmap write page mem[addr,size]
	clear waitmap to supplied state
*/
void Z80::unmapWom(uint16 addr, uint16 size, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfass wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);
	uint16* w = nowritepage - addr;
	XXXASSERT(size_t(w) + addr == size_t(nowritepage)); // fuckin' gcc p'ple

	for (; a <= e; a++)
	{
		a->core_w = w;
		w -= CPU_PAGESIZE;
		a->waitmap_w	  = waitmap;
		a->waitmap_w_size = wm_size;
	}
}

/*	unmap read page mem[addr,size]
	clear waitmap to supplied state
*/
void Z80::unmapRom(uint16 addr, uint16 size, const uint8* waitmap, int wm_size)
{
	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	XXTRAP(addr & CPU_PAGEMASK);  // addr muss in's 'CPU_PAGESIZE' raster passen
	XXTRAP(size & CPU_PAGEMASK);  // addr+size ebenfass wieder
	XXTRAP(size > 0xffff - addr); // wrap around address space end

	PgInfo* a = page + (addr >> CPU_PAGEBITS);
	PgInfo* e = a + (size >> CPU_PAGEBITS);
	uint16* r = noreadpage - addr;
	XXXASSERT(size_t(r) + addr == size_t(noreadpage)); // fuckin' gcc p'ple

	for (; a <= e; a++)
	{
		a->core_r = r;
		r -= CPU_PAGESIZE;
		a->waitmap_r	  = waitmap;
		a->waitmap_r_size = wm_size;
	}
}

/* unmap memory mem[addr,size] with "no memory" and "no waitmap"
 */
void Z80::unmapRam(uint16 addr, uint16 size, const uint8* waitmap, int wm_size)
{
	unmapRom(addr, size, waitmap, wm_size);
	unmapWom(addr, size, waitmap, wm_size);
}


/* unmap memory a[size] with "no memory" and "no waitmap"
   use this if the exact paging location can not be determined easily.
   ((deprecated))
*/
void Z80::unmapMemory(uint16* a, uint32 size)
{
	uint16* e = a + size;
	uint16* p;

	for (int32 i = 0; i < 0x10000; i += CPU_PAGESIZE)
	{
		p = rdPtr(i);
		if (p >= a && p < e)
		{
			PgInfo& p		 = page[i >> CPU_PAGEBITS];
			p.core_r		 = noreadpage - i;
			p.waitmap_r		 = NULL;
			p.waitmap_r_size = 0;
		}
		p = wrPtr(i);
		if (p >= a && p < e)
		{
			PgInfo& p		 = page[i >> CPU_PAGEBITS];
			p.core_w		 = nowritepage - i;
			p.waitmap_w		 = NULL;
			p.waitmap_w_size = 0;
		}
	}
}


// ===========================================================================
// ====	The Z80 Engine =======================================================
// ===========================================================================


// ----	jumping -----------------------------------------------------------
#define LOOP goto nxtcmnd // LOOP to next instruction
#define POKE_AND_LOOP(W, C)                                                                                            \
  {                                                                                                                    \
	w = W;                                                                                                             \
	c = C;                                                                                                             \
	goto poke_and_nxtcmd;                                                                                              \
  }					// POKE(w,c) and goto next instr.
#define EXIT goto x // exit from cpu

// ----	load and store local variables from/to global registers ------------
#define LOAD_REGISTERS                                                                                                 \
  r	 = registers.r;	 /* refresh counter R		*/                                                                          \
  cc = cpu_cycle;	 /* cpu cycle counter		*/                                                                          \
  ic = instr_cnt;	 /* cpu instruction counter	*/                                                                     \
  pc = registers.pc; /* program counter PC		*/                                                                         \
  ra = registers.a;	 /* register A				*/                                                                               \
  rf = registers.f;	 /* register F				*/

#define SAVE_REGISTERS                                                                                                 \
  registers.r  = (registers.r & 0x80) | (r & 0x7f); /* refresh counter R	*/                                            \
  cpu_cycle	   = cc;								/* cpu cycle counter	*/                                            \
  instr_cnt	   = ic;								/* cpu instr. counter	*/                                           \
  registers.pc = pc;								/* program counter PC	*/                                           \
  registers.a  = ra;								/* register A			*/                                                 \
  registers.f  = rf;								/* register F			*/


/* ====	The Z80 ENGINE ====================================================
 */
int Z80::run(int32 cc_max, int32 ic_max, uint32 options)
{
	// LogLine("Run: ccmax=%li icmax=%li, cc=%li, inta=%li, inte=%li",cc_max,ic_max,cpu_cycle,cc_irpt_on,cc_irpt_off);
	XXTRAP((uint8)options != 0);

	int32 cc_maxx = cc_max;
	int32 cc_crtc = 0; // if(options&cpu_crtc) crtc->GetCpuCycleOfNextCrtRead();

	int			   result = 0; // return 0: T cycle count expired
	register int32 cc;		   // cpu cycle counter
	int32		   ic;		   // cpu instruction counter

	register uint16 pc; // z80 program counter
	uint8			ra; // z80 a register
	register uint8	rf; // z80 flags
	register uint8	r;	// z80 r register bit 0...6

	uint16* rzp;  // pointer to IX or IY after 0xDD or 0xFD
#define rz (*rzp) // IX or IY
#ifdef _BIG_ENDIAN
  #define rzh (((uint8*)rzp)[0]) // XH or YH
  #define rzl (((uint8*)rzp)[1]) // XL or YL
#else
  #define rzh (((uint8*)rzp)[1]) // XH or YH
  #define rzl (((uint8*)rzp)[0]) // XL or YL
#endif

	uint8 c = 0; // general purpose byte register

	uint16 w;		 // general purpose word register
#define wl (uint8) w // access low byte of w
#define wh (w >> 8)	 // access high byte of w

	uint16 wm;		   // scratch for macro internal use
#define wml (uint8) wm // access low byte of wm
#define wmh (wm >> 8)  // access high byte of wm


// load local variables from global registers:
restart:
	LOAD_REGISTERS;


	// ---- NMI TEST ---------------

slow_loop:

	// test non-maskable interrupt:
	// the NMI is edge-triggered and automatically cleared

	if (cc >= cc_nmi)
	{ // 11 T cycles, probably: M1:5T, M2:3T, M3:3T
		if (peek(pc) == HALT)
			pc++;		 // TODO: HALT and NMI
						 //	IFF2 = IFF1;					// save current irpt enable bit
		IFF1 = disabled; // disable irpt, preserve IFF2
		r += 1;
		cc += 5;			  // M1: 5 T: interrupt acknowledge cycle
		Z80_INFO_NMI(cc - 2); // clear or re-trigger nmi (in macro)
		PUSH(pc >> 8);		  // M2: 3 T: push pch
		PUSH(pc);			  // M3: 3 T: push pcl
		pc = 0x0066;		  // jump to 0x0066
	}
	if (cc_max > cc_nmi) { cc_max = cc_nmi; }


	// ---- INTERRUPT TEST -----------------

	// test maskable interrupt:
	// note: the /INT signal is not cleared by int ack
	//		 the /INT signal is sampled once per instruction at the end of instruction, during refresh cycle
	//		 if the interrupt is not started until the /INT signal goes away then it is lost!

	if (cc < cc_irpt_on) // int still ahead?
	{
		cc_max = Min(cc_max, cc_irpt_on);
		LOOP;
	}
	else if (cc >= cc_irpt_off) // int already expired?
	{
		LOOP;
	}
	else if (IFF1 == disabled) // irpt disabled?
	{
		LOOP;
	}
	else // handle interrupt
	{
		if (peek(pc) == HALT) pc++; // TODO: HALT and Interrupt
		IFF1 = IFF2 = disabled;		// disable interrupt
		r += 1;						// M1: 2 cc + standard opcode timing (min. 4 cc)
		cc += 6;					// /HALT test and busbyte read in cc+4
		Z80_INFO_IRPT(cc - 2);		// c = busbyte (in macro)	w = busbytes read in M2+M3 cycle (if c==CALL && mode 0)

		switch (registers.im)
		{
		case 0:
			/*	mode 0: read instruction from bus
				NOTE:	docs say that M1 is always 6 cc in mode 0, but that is not conclusive:
						the 2 additional T cycles are before opcode fetch, and thus they must always add to instruction
			   execution the timing from the moment of instruction fetch (e.g. cc +2 in a normal M1 cycle, cc +4 in int
			   ack M1 cycle) and can't be shortended. to be tested somehow.	kio 2004-11-12
			*/
			switch (c)
			{
			case RST00:
			case RST08: //	timing: M1: 2+5 cc: opcode RSTxx  ((RST is 5 cc))
			case RST10:
			case RST18: //			M2:	3 cc:   push pch
			case RST20:
			case RST28: //			M3: 3 cc:   push pcl
			case RST30:
			case RST38:
				cc += 1;
				w = c - RST00;
				goto irpt_xw;

			case CALL:		  //	timing:	M1:   2+4 cc: opcode CALL
				cc += 7;	  //			M2+3: 3+4 cc: get NN			provided in w (by macro Z80_INFO_IRPT)
				goto irpt_xw; //			M4+5: 3+3 cc: push pch, pcl

			default:	//	only RSTxx and CALL NN are supported
				TODO(); //  any other opcode is of no real use.
			};

		case 1:
			/*	Mode 1:	RST38
				NOTE:	docs say, timing is 13 cc for implicit RST38 in mode 1 and 12 cc for fetched RST38 in mode 0.
						maybe it is just vice versa? in mode 1 the implicitely known RST38 can be executed with the
			   start of the M1 cycle and finish after 5 cc, prolonged to the irpt ackn M1 cycle min. length of 6 cc.
			   Currently i'll stick to 13 cc as doc'd. to be tested somehow.	kio 2004-11-12
				TODO:	does the cpu a dummy read in M1?
						at least ZX Spectrum videoram waitcycles is no issue because irpt is off-screen.
			*/
			cc += 1;	  //	timing:	M1:	7 cc: int ack cycle (as doc'd)
			w = 0x0038;	  //			M2: 3 cc: push pch
			goto irpt_xw; //			M3: 3 cc: push pcl

		irpt_xw:
			PUSH(pc >> 8); //	M2: 3 cc: push pch
			PUSH(pc);	   //	M3: 3 cc: push pcl and load pc
			pc = w;
			LOOP;

		case 2:
			/*	Mode 2:	jump via table
			 */
			cc += 1;	   //	timing:	M1: 7 cc: int ack  and  read interrupt vector from bus
			PUSH(pc >> 8); //			M2: 3 cc: push pch
			PUSH(pc);	   //			M3: 3 cc: push pcl
			w = registers.i * 256 + c;
			PEEK(PCL, w);	  //			M4: 3 cc: read low byte from table
			PEEK(PCH, w + 1); //			M5: 3 cc: read high byte and jump
			pc = PC;
			LOOP;

		default: IERR(); // bogus irpt mode
		};
	}
	IERR();


	// ----	MAIN INSTRUCTION DISPATCHER --------------------------------

poke_and_nxtcmd:
	POKE(w, c); // --> CPU_WAITCYCLES, CPU_READSCREEN, cc+=3, Poke(w,c)

nxtcmnd:
	if (cc < cc_max && ic < ic_max) // fast loop exit test
	{
	loop_ei:
#include "Z80codes.h"
		IERR();
	}


	// ---- EXIT TEST ------------------------------------------------

	//	If a macro whishes to exit RunCpu() after finishing the current opcode:
	//		result = my_result_code
	//		ic_max = 0
	//	If a macro whishes to do the Int/Nmi tests after the current opcode:
	//		cc_max = cc
	//	If a macro whishes to exit immediately without executing the current opcode:
	//		result = my_result_code
	//		undo whatever must be undone. depends on macro position...
	//		EXIT		((goto x))

	cc_max = cc_maxx;
	if (cc < cc_max && ic < ic_max)
	{
		goto slow_loop; // nmi + irpt test
	}


	//	cc >= cc_max && result == 0		-> T cycle count expired		-> return 0
	//	ic >= ic_max && result == 0		-> instruction count expired	-> return 1
	//	ic >= ic_max && result >= 2		-> macro forced exit			-> return result

	if (result == 0) result = ic >= ic_max;
	goto x;


	// ---- EXIT ----

x:
	SAVE_REGISTERS;
	return result;
}
