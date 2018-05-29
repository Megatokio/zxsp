/*	Copyright  (c)	Günter Woigk 1996 - 2018
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


	Z80 Emulator
	originally based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	ED instruction handlers
*/


#ifndef Z80_H
void xxx(){		// keep the syntax checker happy
#endif


GET_ED_OP(c);

switch ( c )
{

case ADC_HL_BC: rzp=&registers.bc; goto adc_hl_rr;		// timing: pc:4, pc+1:11
case ADC_HL_DE: rzp=&registers.de; goto adc_hl_rr;
case ADC_HL_HL: rzp=&registers.hl; goto adc_hl_rr;
case ADC_HL_SP: rzp=&registers.sp; goto adc_hl_rr;
	 adc_hl_rr:	cc+=7; M_ADCW(rz); LOOP;

case SBC_HL_BC: rzp=&registers.bc; goto sbc_hl_rr;		// timing: pc:4, pc+1:1
case SBC_HL_DE: rzp=&registers.de; goto sbc_hl_rr;
case SBC_HL_HL: rzp=&registers.hl; goto sbc_hl_rr;
case SBC_HL_SP: rzp=&registers.sp; goto sbc_hl_rr;
	 sbc_hl_rr:	cc+=7; M_SBCW(rz); LOOP;

case LD_xNN_BC:	rzp = &registers.bc; goto ld_xnn_rr;		// timing: pc:4, pc+1:4, pc+2:3, pc+3:3, nn:3, nn+1:3
case LD_xNN_DE:	rzp = &registers.de; goto ld_xnn_rr;
case ED_xNN_HL:	rzp = &registers.hl; goto ld_xnn_rr;
case LD_xNN_SP:	rzp = &registers.sp; goto ld_xnn_rr;
	 ld_xnn_rr:	GET_NN(w); POKE(w,rzl); POKE_AND_LOOP(w+1,rzh);

case LD_BC_xNN:	GET_NN(w); PEEK(RC,w);  PEEK(RB,w+1); LOOP;		// timing: pc:4, pc+1:4, pc+2:3, pc+3:3, nn:3, nn+1:3
case LD_DE_xNN:	GET_NN(w); PEEK(RE,w);  PEEK(RD,w+1); LOOP;
case ED_HL_xNN:	GET_NN(w); PEEK(RL,w);  PEEK(RH,w+1); LOOP;
case LD_SP_xNN:	GET_NN(w); PEEK(SPL,w); PEEK(SPH,w+1);LOOP;

{ uint8* p;
  uint8 z;
case IN_F_xC:	p=&z;  goto in_r_xc;		// timing: pc:4, pc+1:4, IO
case IN_B_xC:	p=&RB; goto in_r_xc;
case IN_C_xC:	p=&RC; goto in_r_xc;
case IN_D_xC:	p=&RD; goto in_r_xc;
case IN_E_xC:	p=&RE; goto in_r_xc;
case IN_H_xC:	p=&RH; goto in_r_xc;
case IN_L_xC: 	p=&RL; goto in_r_xc;
	 in_r_xc:	M_IN(*p); LOOP;
case IN_A_xC:	M_IN(ra); LOOP;
}

case OUT_xC_B:	c=RB; goto out_xc_r;		// timing: pc:4, pc+1:4, IO
case OUT_xC_C: 	c=RC; goto out_xc_r;
case OUT_xC_D: 	c=RD; goto out_xc_r;
case OUT_xC_E: 	c=RE; goto out_xc_r;
case OUT_xC_H: 	c=RH; goto out_xc_r;
case OUT_xC_L: 	c=RL; goto out_xc_r;
case OUT_xC_A: 	c=ra; goto out_xc_r;
	 out_xc_r:	OUTPUT(BC,c); LOOP;
case OUT_xC_0: 	c=0; Z80_INFO_ILLEGAL(cc-8,pc-2); goto out_xc_r;

case ED4E:		// illegal: im0
case ED66:
case ED6E:		Z80_INFO_ILLEGAL(cc-8,pc-2);
case IM_0:		registers.im=0; LOOP;
case ED76:		Z80_INFO_ILLEGAL(cc-8,pc-2);	// illegal: im1
case IM_1:		registers.im=1; LOOP;
case ED7E:		Z80_INFO_ILLEGAL(cc-8,pc-2);	// illegal: im2
case IM_2:		registers.im=2; LOOP;

case ED4C:		// illegal NEG
case ED54:
case ED5C:
case ED64:
case ED6C:
case ED74:
case ED7C:		Z80_INFO_ILLEGAL(cc-8,pc-2);
case NEG:   	c=ra; ra=0; M_SUB(c); LOOP;



case LD_I_A:   	cc+=1; RI=ra;				 Z80_INFO_LD_I_A; LOOP;		// timing: pc:4, pc+1:5
case LD_R_A:   	cc+=1; registers.r = r = ra; Z80_INFO_LD_R_A; LOOP;

case LD_A_I:	cc+=1;							// timing: pc:4, pc+1:5
				ra	= RI;
				rf	= (rf&C_FLAG) + (IFF2?P_FLAG:0) + (ra?0:Z_FLAG) + (ra&S_FLAG);
				LOOP;

case LD_A_R:	cc+=1;							// timing: pc:4, pc+1:5
				ra = (registers.r&0x80) + (r&0x7F);
				rf	= (rf&C_FLAG) + (IFF2?P_FLAG:0) + (ra?0:Z_FLAG) + (ra&S_FLAG);
				LOOP;

/*	RETI, RETN and illegal variants:
	They all copy iff2 -> iff1, like documented for RETN (source: z80-documented.pdf)
	Whether any of these opcodes is recognized as RETI solely depends on the peripherial IC (namely the Z80 PIO)
	whether it decodes that opcode as 'RETI'.
*/
case RETI:	Z80_INFO_RETI;						// timing: pc:4, pc+1:4, sp:3, sp+1:3
			IFF1=IFF2;							// lt. z80-documented.pdf: all RETN _and_ all RETI copy iff2 -> iff1
			goto ret;

case ED5D:	// illegal: retn
case ED6D:
case ED7D:
case ED55:
case ED65:
case ED75:	Z80_INFO_ILLEGAL(cc-8,pc-2);		// timing: pc:4, pc+1:4, sp:3, sp+1:3
case RETN:	Z80_INFO_RETN;
			IFF1=IFF2;
			goto ret;

{ uint8 o;
case RRD:	w=HL; PEEK(o,w);					// timing: pc:4, pc+1:4, hl:3,4*1, hl:3
			c	= (o>>4) + (ra<<4);
			ra	= (ra&0xF0) + (o&0x0F);
			goto rld;

case RLD:	w=HL; PEEK(o,w);					// timing: pc:4, pc+1:4, hl:3,4*1, hl:3
			c 	= (o<<4) + (ra&0x0F);
			ra	= (ra&0xF0)+(o>>4);
	 rld:	rf	= (rf&C_FLAG) + zlog_flags[ra];
			SKIP_4X1CC(w);						// HL (kio-tested-2005-01-09)
			POKE_AND_LOOP(w,c);
}


// ########	Block Instructions ###############################

//			26.jun.2000 kio: LDIR etc. _are_ interruptable!
//			block commands are implemented as the z80 do them:
//			do the instruction once and decrement the pc if not yet finished
//			so that the next M1 cycle will fetch the same block instruction again.
//			=>	interruptable
//				proper T cycle timing and r register emulation
//				proper malbehavior when block commands start overwriting itself

case LDDR:	w = uint16(-1); goto ldir;			// Load, decrement and repeat
case LDIR:	w = 1;								// Load, increment and repeat
ldir:		PEEK(c,HL);							// timing:  pc:4, pc+1:4, hl:3, de:3,2x1 [de:5x1]		kio tested 2005-01-14/15
			POKE(DE,c);
			rf &= ~(N_FLAG+H_FLAG+P_FLAG);
			if (--BC) { rf |= P_FLAG; pc-=2; SKIP_7X1CC(DE); } else { SKIP_2X1CC(DE); } 	// DE before ++/--
			DE+=w; HL+=w;
			LOOP;

case LDD:	w = uint16(-1); goto ldi;			// Load and decrement
case LDI:	w = 1;								// Load and increment
ldi:		PEEK(c,HL); HL+=w;					// timing:  pc:4, pc+1:4, hl:3, de:3,2x1		kio tested 2005-01-14/15
			POKE(DE,c);
			SKIP_2X1CC(DE);						// DE before incremented (kio-tested-2005-01-09)
			DE+=w;
			rf &= ~(N_FLAG+H_FLAG+P_FLAG);
			if (--BC) rf |= P_FLAG;
			LOOP;


case CPDR:	w = HL--; goto cpir;				// Compare, decrement and repeat
case CPIR:	w = HL++;							// Compare, increment and repeat
cpir:		PEEK(c,w); c = ra - c;				// timing: pc:4, pc+1:4, hl:3,5*1,[5*1]		kio verified 2005-01-10
			SKIP_5X1CC(w);
			BC -= 1;
			rf	= (rf&C_FLAG) + (c&S_FLAG) + (c?0:Z_FLAG) + N_FLAG + (BC?P_FLAG:0) + ((ra^(ra-c)^c)&H_FLAG);
			if (BC&&c) { pc-=2; SKIP_5X1CC(w); }	// LOOP not yet finished
			LOOP;

case CPD:	w = HL--; goto cpi;					// Compare and decrement
case CPI:	w = HL++;							// Compare and increment
cpi:		PEEK(c,w); c = ra - c;				// timing: pc:4, pc+1:4, hl:3,5*1
			SKIP_5X1CC(w);
			BC -= 1;
			rf	= (rf&C_FLAG) + (c&S_FLAG) + (c?0:Z_FLAG) + N_FLAG + (BC?P_FLAG:0) + ((ra^(ra-c)^c)&H_FLAG);
			LOOP;


case INDR:	w = HL--; goto inir;				// input, decrement and repeat
case INIR:	w = HL++;							// input, increment and repeat
inir:		cc+=1;								// timing: pc:4, pc+1:5, IO, hl:3,[5*1]
			INPUT(BC,c);
			POKE(w,c);
			if (--RB) { pc-=2; SKIP_5X1CC(w); }	// LOOP not yet finished
			rf = N_FLAG + (RB?0:Z_FLAG);		// TODO: INIR etc.: flags checken
			LOOP;

case IND:	w = HL--; goto ini;					// input and decrement
case INI:	w = HL++;							// input and increment
ini:		cc+=1;								// timing: pc:4, pc+1:5, IO, hl:3
			INPUT(BC,c);
			rf = N_FLAG + (--RB?0:Z_FLAG);
			POKE_AND_LOOP(w,c);


case OTDR:	w = HL--; goto otir;				// output, decrement and repeat
case OTIR:	w = HL++;							// output, increment and repeat
otir:		cc+=1;								// timing:  pc:4, pc+1:5, hl:3, IO, [hl:5*1]
			PEEK(c,w);
			--RB;								// kio 2005-11-18: OUTI: decr B before putting it on the bus [post on css by Alvin Albrecht]
			OUTPUT(BC,c);
			if (RB) { pc-=2; SKIP_5X1CC(w); }
			rf = N_FLAG + (RB?0:Z_FLAG);
			LOOP;

case OUTD:	w = HL--; goto outi;				// output and decrement
case OUTI:	w = HL++;							// output and increment
outi:		cc+=1;								// timing: pc:4, pc+1:5, hl:3, IO
			PEEK(c,w);
			--RB;								// kio 2005-11-18: OUTI: decr B before putting it on the bus [post on css by Alvin Albrecht]
			OUTPUT(BC,c);
			rf = N_FLAG + (RB?0:Z_FLAG);
			LOOP;


// ---------------------------------------------

default:	// ED77, ED7F, ED00-ED3F and ED80-EDFF except block instr
			Z80_INFO_ILLEGAL(cc-8,pc-2);
			LOOP;
};



#ifndef Z80_H
}
#endif

