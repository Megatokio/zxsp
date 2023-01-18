/*	Copyright  (c)	Günter Woigk 1996 - 2019
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


	Z80 Emulator
	originally based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	macro definitions
*/


// access z80 registers:

#define	IFF1	registers.iff1		// actually used irpt flip flop
#define	IFF2	registers.iff2		// copy of iff1 during nmi processing
#define	RR		registers.r			// 7 bit DRAM refresh counter
#define	RI		registers.i			// hi byte of interrupt vector: i register

#define	RA		registers.a
#define	RF		registers.f
#define	RB		registers.b
#define	RC		registers.c
#define	RD		registers.d
#define	RE		registers.e
#define	RH		registers.h
#define	RL		registers.l

#define	RA2		registers.a2
#define	RF2		registers.f2

#define	XH		registers.xh
#define	XL		registers.xl
#define	YH		registers.yh
#define	YL		registers.yl
#define	PCH		registers.pch
#define	PCL		registers.pcl
#define	SPH		registers.sph
#define	SPL		registers.spl

#define	BC		registers.bc
#define	DE		registers.de
#define	HL		registers.hl

#define	BC2		registers.bc2
#define	DE2		registers.de2
#define	HL2		registers.hl2

#define	IX		registers.ix
#define	IY		registers.iy
#define	PC		registers.pc
#define	SP		registers.sp



#define	CC_WAIT_R(CC)	cc += pg.waitmap_r[ (CC) % pg.waitmap_r_size ]
#define	CC_WAIT_W(CC)	cc += pg.waitmap_w[ (CC) % pg.waitmap_w_size ]


#define OUTPUT(A,R)											\
{															\
	cpu_cycle = cc+=3; 										\
	machine->outputAtCycle(cc,A,R);  /* cc of bus cycle */	\
	cc = cpu_cycle+1; 										\
	cc_max = min(cc_max,cc_nmi); 							\
}

#define INPUT(A,R)			 								\
{															\
	cpu_cycle = cc+=3;  									\
	R = machine->inputAtCycle(cc,A); /* cc of bus cycle */	\
	cc = cpu_cycle+1;  										\
}


#define	PEEK(R,A)							\
{											\
	PgInfo& pg = getPage(A);				\
	z32 = pg.both_r(A);						\
	c = uint8(z32);							\
	if( (z32&=options) )					\
	{										\
		if(z32&cpu_break_r) { break_addr=A; result=cpu_exit_r; ic_max=0; }	\
		if(z32&cpu_waitmap) { CC_WAIT_R(cc+2); }							\
		if(z32&cpu_r_access){ pg.both_r(A) -= cpu_r_access; }				\
		if(z32&cpu_floating_bus){ c=machine->ula->getFloatingBusByte(cc); } \
		if(z32&cpu_memmapped_r) { c=machine->readMemMappedPort(cc+2,A,c); }	\
	};	cc+=3;								\
	R = c;									\
}

/*	write byte to ram:
	unconditionally write to primary page_w
	write to secondary page_w2 if not null
	wait cycles are only added for primary page
*/
#define	POKE(A,R)							\
{											\
	cc += 2;								\
	PgInfo& pg = getPage(A);				\
	if((z32 = pg.both_w(A) & options))		\
	{										\
		if(z32&cpu_waitmap) { CC_WAIT_W(cc); }													\
		if(z32&cpu_break_w) { break_addr=A; result=cpu_exit_w; ic_max=0; }						\
		if(z32&cpu_crtc)	{ if(cc >= cc_crtc) cc_crtc = crtc->updateScreenUpToCycle(cc); }	\
		if(z32&cpu_w_access){ pg.both_w(A) -= cpu_w_access; }									\
		if(z32&cpu_memmapped_w) { machine->writeMemMappedPort(cc,A,R); }						\
	};										\
	if(pg.core_w2)							\
	{										\
		if((z32 = pg.both_w2(A) & options))	\
		{									\
		if(z32&cpu_break_w) { break_addr=A; result=cpu_exit_w; ic_max=0; }						\
		if(z32&cpu_crtc)	{ if(cc >= cc_crtc) cc_crtc = crtc->updateScreenUpToCycle(cc); }	\
		if(z32&cpu_w_access){ pg.both_w(A) -= cpu_w_access; }									\
		if(z32&cpu_memmapped_w) { machine->writeMemMappedPort(cc,A,R); }						\
		}									\
		pg.data_w2(A) = R;					\
	}										\
	pg.data_w(A) = R;						\
	cc+=1;									\
}

#define PEEK_INSTR(R)                       \
{											\
	PgInfo& pg = getPage(pc);				\
	z32 = pg.both_r(pc);					\
	R = uint8(z32);							\
	if( (z32&options&cpu_crtc_zx81) && (pc&0x8000) )	\
	{ R = peek(pc&0x7fff); if(~R&0x40) R = NOP; };		\
}

#define	GET_INSTR(R)						\
{											\
	PgInfo& pg = getPage(pc);				\
	z32 = pg.both_r(pc);					\
	R = uint8(z32);							\
	if((z32 &= options))					\
	{										\
		if(z32&cpu_break_x)	{ if(machine->break_ptr==&pg.both_r(pc)) { machine->break_ptr=nullptr; } else	\
							{ break_addr=pc; result=cpu_exit_x; EXIT; } }									\
		if(z32&cpu_patch)   { SAVE_REGISTERS; R = machine->handleRomPatch(pc,R); LOAD_REGISTERS;}			\
		if(z32&cpu_crtc_zx81 && pc&0x8000)																	\
							{ R = peek(pc&0x7fff);															\
							  if(~R&0x40) { UlaMonoPtr(crtc)->crtcRead(cc,R); R = NOP; }					\
							  if(~r&0x40) { cc_irpt_on=cc+2; cc_irpt_off=cc+5; if(IFF1==enabled) cc_max=cc+2; }	\
							  else        { cc_irpt_on=0x7fffffff; } }										\
		if(z32&cpu_waitmap)	{ CC_WAIT_R(cc+2); }															\
		if(z32&cpu_x_access){ pg.both_r(pc) -= cpu_x_access; }												\
	};	ic+=1; cc+=4; pc+=1; r+=1;			\
}

/*	get next byte (ip++)
*/
#define	GET_BYTE(RGL)						\
{											\
	PgInfo& pg = getPage(pc);				\
	z32 = pg.both_r(pc);					\
	RGL uint8(z32);							\
	if((z32&=options))						\
	{										\
	 /*	if(z32&cpu_break_x) { break_addr=pc; result=cpu_exit_x; ic_max=0; }	*/	\
		if(z32&cpu_waitmap) { CC_WAIT_R(cc+2); }								\
		if(z32&cpu_x_access){ pg.both_r(pc) -= cpu_x_access; }					\
	};	pc+=1;								\
}

#define	GET_N(R)		{ GET_BYTE(R=);		  cc+=3; }
#define	SKIP_N()		{ GET_BYTE((void));	  cc+=3; }
#define	GET_XYCB_OP(R)	{ GET_BYTE(R=);		  cc+=3; SKIP_2X1CC(pc-1); }
#define	GET_ED_OP(R)	{ GET_BYTE(R=); r+=1; cc+=4; ic+=1; }				// ic+=1 -> .rzx support
#define	GET_XY_OP(R)	{ GET_BYTE(R=); r+=1; cc+=4; ic+=1; }				// ic+=1 -> .rzx support
#define	GET_CB_OP(R)	{ GET_BYTE(R=); r+=1; cc+=4; ic+=1; }				// ic+=1 -> .rzx support


/*	1 cc with wait test
	call		pc:4, pc+1:3, pc+2:3,1, sp-1:3, sp-2:3
	dec (hl)	pc:4, hl:3,1, hl:3
	dec (ix+d)	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3,1, ix+n:3
	ex hl,(sp)	pc:4, sp:3, sp+1:3,1, sp+1:3, sp:3,2x1
	CB opcode:  peek(hl): hl:3,1
	XYCB bit:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1
	XYCB rlc:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1, ix+n:3
	XYCB set:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1, ix+n:3
*/
#define	SKIP_1CC(A)							\
{											\
	cc += 1;								\
	if(options&cpu_ula_sinclair)			\
	{										\
		PgInfo& pg = getPage(A);			\
		z32 = pg.both_r(A);					\
		if( z32&options&cpu_waitmap )		\
		{									\
			CC_WAIT_R(cc+1);				\
		}									\
	}										\
}


/*	2 cc with wait test
	ex hl,(sp)	pc:4, sp:3, sp+1:3,1, sp+1:3, sp:3,2x1
	ex ix,(sp)	pc:4, pc+1:4, sp:3, sp+1:3,1, sp+1:3, sp:3,2x1
	ldir		pc:4, pc+1:4, hl:3, de:3,2x1 [de:5x1]
	ldi			pc:4, pc+1:4, hl:3, de:3,2x1
	ld (ix),n	pc:4, pc+1:4, pc+2:3, pc+3:3,2x1, ix+n:3
	GET_XYCB_OP
*/
#define	SKIP_2X1CC(A)						\
{											\
	cc += 2;								\
	if(options&cpu_ula_sinclair)			\
	{										\
		PgInfo& pg = getPage(A);			\
		z32 = pg.both_r(A);					\
		if( z32&options&cpu_waitmap )		\
		{									\
			CC_WAIT_R(cc+0);				\
			CC_WAIT_R(cc+1);				\
		}									\
	}										\
}


/*	4 cc with wait test in all cycles
	rld		pc:4, pc+1:4, hl:3,4*1, hl:3
	rrd		pc:4, pc+1:4, hl:3,4*1, hl:3
*/
#define	SKIP_4X1CC(A)						\
{											\
	cc += 4;								\
	if(options&cpu_ula_sinclair)			\
	{										\
		PgInfo& pg = getPage(A);			\
		z32 = pg.both_r(A);					\
		if( z32&options&cpu_waitmap )		\
		{									\
			CC_WAIT_R(cc-2);				\
			CC_WAIT_R(cc-1);				\
			CC_WAIT_R(cc-0);				\
			CC_WAIT_R(cc+1);				\
		}									\
	}										\
}


/*	5 cc with wait test in all cycles
	jr			pc:4, pc+1:3,5*1
	cpi			pc:4, pc+1:4, hl:3,5*1
	cpir		pc:4, pc+1:4, hl:3,5*1, [5*1]
	inir		pc:4, pc+1:5, IO, hl:3, [5*1]
	otir		pc:4, pc+1:5, hl:3, IO, [5*1]
	ld (ix+d),r	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
	ld r,(ix+d)	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
	dec (ix+d)	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3,1, ix+n:3
	cp a,(ix+d)	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
*/
#define	SKIP_5X1CC(A)						\
{											\
	cc += 5;								\
	if(options&cpu_ula_sinclair)			\
	{										\
		PgInfo& pg = getPage(A);			\
		z32 = pg.both_r(A);					\
		if( z32&options&cpu_waitmap )		\
		{									\
			CC_WAIT_R(cc-3);				\
			CC_WAIT_R(cc-2);				\
			CC_WAIT_R(cc-1);				\
			CC_WAIT_R(cc+0);				\
			CC_WAIT_R(cc+1);				\
		}									\
	}										\
}


/*	7 cc with wait test in all cycles
	ldir	pc:4, pc+1:4, hl:3, de:3,2x1 [de:5x1]
*/
#define	SKIP_7X1CC(A)						\
{											\
	cc += 7;								\
	if(options&cpu_ula_sinclair)			\
	{										\
		PgInfo& pg = getPage(A);			\
		z32 = pg.both_r(A);					\
		if( z32&options&cpu_waitmap )		\
		{									\
			CC_WAIT_R(cc-5);				\
			CC_WAIT_R(cc-4);				\
			CC_WAIT_R(cc-3);				\
			CC_WAIT_R(cc-2);				\
			CC_WAIT_R(cc-1);				\
			CC_WAIT_R(cc-0);				\
			CC_WAIT_R(cc+1);				\
		}									\
	}										\
}


#define	GET_NN(RR)			{ GET_N(RR); uint16 w_nn; GET_N(w_nn); RR += 256*w_nn; }
#define	POP(R)				{ PEEK(R,SP); SP++; }
#define	PUSH(R)				{ --SP; POKE(SP,R); }


// hooks:

#define	Z80_INFO_EX_HL_xSP	Z80_INFO_POP										/* called after opcode execution */
#define	Z80_INFO_RET		Z80_INFO_POP										/* called after opcode execution */
#define	Z80_INFO_POP		if(options&cpu_break_sp && SP==stack_breakpoint)	/* called after opcode execution */  \
							{ break_addr=SP; result=cpu_exit_sp; ic_max=0; }

#define	Z80_INFO_RETI			/* nop */
#define	Z80_INFO_RETN			/* nop */
#define	Z80_INFO_HALT			/* nop */
#define Z80_INFO_ILLEGAL(CC,PC)	/* nop */
#define Z80_INFO_RST00			/* nop */
#define Z80_INFO_RST08			/* nop */
#define Z80_INFO_RST10			/* nop */
#define Z80_INFO_RST18			/* nop */
#define Z80_INFO_RST20			/* nop */
#define Z80_INFO_RST28			/* nop */
#define Z80_INFO_RST30			/* nop */
#define Z80_INFO_RST38			/* nop */
#define Z80_INFO_EI				/* nop */
#define Z80_INFO_LD_R_A			/* nop */
#define Z80_INFO_LD_I_A			/* nop */




// --------------------------------------------------------------------
// ----	INSTRUCTION MACROS --------------------------------------------
// --------------------------------------------------------------------


/*	RLC ... SRL:	set/clr C, Z, P, S;
			clear	N=0, H=0
			pres.	none
*/
#define M_RLC(R)      				\
	rf  = R>>7;						\
	R   = (R<<1)+rf; 				\
	rf |= zlog_flags[R]

#define M_RRC(R)      				\
	rf  = R&0x01;					\
	R   = (R>>1)+(rf<<7);		 	\
	rf |= zlog_flags[R]

#define M_RL(R)						\
	if (R&0x80)						\
	{	R 	= (R<<1)+(rf&0x01);		\
		rf	= zlog_flags[R]+C_FLAG;	\
	} else							\
	{	R 	= (R<<1)+(rf&0x01);		\
		rf	= zlog_flags[R];		\
	}

#define M_RR(R)						\
	if (R&0x01)						\
	{	R 	= (R>>1)+(rf<<7);		\
		rf	= zlog_flags[R]+C_FLAG;	\
	} else							\
	{	R 	= (R>>1)+(rf<<7);		\
		rf	= zlog_flags[R];		\
	}

#define M_SLA(R)					\
	rf	= R>>7;						\
	R <<= 1;						\
	rf |= zlog_flags[R]

#define M_SRA(R)					\
	rf	= R&0x01;					\
	R	= (R&0x80)+(R>>1);			\
	rf |= zlog_flags[R]

#define M_SLL(R)					\
	rf	= R>>7;						\
	R	= (R<<1)+1;					\
	rf |= zlog_flags[R]

#define M_SRL(R)					\
	rf	= R&0x01;					\
	R >>= 1;						\
	rf |= zlog_flags[R]


/*	BIT:	set/clr	Z
			clear	N=0, H=1
			pres	C
			takes other flags from corresponding bits in tested byte!
*/
#define M_BIT(N,R)								\
	rf	= (rf&C_FLAG) + 						\
		  (R&(S_FLAG+P_FLAG)) + 				\
		   H_FLAG + 							\
		  ((R&N)?0:Z_FLAG)


/*	ADD ... CP:	set/clr	Z, S, V, C, N, H
				pres	none
*/
#define M_ADD(R)								\
	wm	= ra+R;									\
	rf	= wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			 + (~(ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			 + ((ra^R^wml)&H_FLAG);				\
	ra	= wml

#define M_SUB(R)								\
	wm	= ra-R;									\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;	\
	ra	= wml

#define M_ADC(R)								\
	wm	= ra+R+(rf&C_FLAG);						\
	rf	= wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			 + (~(ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			 + ((ra^R^wml)&H_FLAG);				\
	ra	= wml

#define M_SBC(R)								\
	wm	= ra-R-(rf&C_FLAG);						\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;	\
	ra	= wml

#define M_CP(R)									\
	wm	= ra-R;									\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;


/*	AND ... XOR:	set/clr	Z, P, S
					clear	C=0, N=0, H=0/1 (OR,XOR/AND)
					pres	none
*/
#define M_AND(R)								\
	ra &= R;									\
	rf	= H_FLAG|zlog_flags[ra]

#define M_OR(R)									\
	ra |= R;									\
	rf	= zlog_flags[ra]

#define M_XOR(R)								\
	ra ^= R;									\
	rf	= zlog_flags[ra]


/*	INC ... DEC:	set/clr	Z,P,S,H
					clear	N=0/1 (INC/DEC)
					pres	C
*/
#define M_INC(R)								\
	R++;										\
	rf	= (rf&C_FLAG) + 						\
		  (R?0:Z_FLAG) + 						\
		  (R&S_FLAG) + 							\
		  (R==0x80?V_FLAG:0) + 					\
		  (R&0x0F?0:H_FLAG)

#define M_DEC(R)								\
	R--;										\
	rf	= (rf&C_FLAG) + 						\
		  (R?0:Z_FLAG) + 						\
		  (R&S_FLAG) + 							\
		  (R==0x7F?V_FLAG:0) + 					\
		  (((R+1)&0x0F)?0:H_FLAG) +				\
		   N_FLAG


/*	ADDW:	set/clr	C
			clear	N=0
			pres	Z, P, S
			unkn	H
*/
#define M_ADDW(R1,R2)							\
	rf &= ~(N_FLAG+C_FLAG);						\
	rf |= ((uint32)R1+(uint32)R2)>>16;			\
	R1 += R2;


/*	ADCW, SBCW:	set/clr	C,Z,V,S
			clear	N=0/1 (ADC/SBC)
			unkn	H
			pres	none
*/
#define M_ADCW(R)								\
	wm	= HL+R+(rf&C_FLAG);						\
	rf	= (((uint32)HL+(uint32)R+(rf&C_FLAG))>>16)\
			+ (wm?0:Z_FLAG) + (wmh&S_FLAG)		\
			+ (~(HL^R)&(wm^HL)&0x8000?V_FLAG:0);\
	HL	= wm

#define M_SBCW(R)								\
	wm	= HL-R-(rf&C_FLAG);						\
	rf	= (((uint32)HL-(uint32)R-(rf&C_FLAG))>>31)\
			+ (wm?0:Z_FLAG) + (wmh&S_FLAG)		\
			+ ((HL^R)&(wm^HL)&0x8000?V_FLAG:0)	\
			+ N_FLAG;							\
	HL	= wm


/*	IN	set/clr	Z, P, S, H
		clear	N=0
		pres	C
*/
#define M_IN(R)									\
	INPUT(BC,R);								\
	rf	= (rf&C_FLAG) + zlog_flags[R]










