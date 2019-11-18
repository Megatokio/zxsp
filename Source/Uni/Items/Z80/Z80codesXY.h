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

	ix/iy instruction handlers


	Timing:
	The replacement of HL by either IX or IY does not affect the timings,
	except for the addition of an initial pc:4 for the DD or FD prefix;
*/


#ifndef Z80_H
void xxx(){		// keep the syntax checker happy
#endif


GET_XY_OP(c);

switch ( c )
{

case LD_H_B:	rzh=RB; goto info_ill2;
case LD_L_B:   	rzl=RB; goto info_ill2;
case LD_H_C:	rzh=RC; goto info_ill2;
case LD_L_C:	rzl=RC; goto info_ill2;
case LD_H_D:	rzh=RD; goto info_ill2;
case LD_L_D:	rzl=RD; goto info_ill2;
case LD_H_E:	rzh=RE; goto info_ill2;
case LD_L_E:	rzl=RE; goto info_ill2;
case LD_B_H:	RB=rzh; goto info_ill2;
case LD_C_H:	RC=rzh; goto info_ill2;
case LD_D_H:	RD=rzh; goto info_ill2;
case LD_E_H:	RE=rzh; goto info_ill2;
case LD_A_H:	ra=rzh; goto info_ill2;
case LD_B_L:	RB=rzl; goto info_ill2;
case LD_C_L:	RC=rzl; goto info_ill2;
case LD_D_L:	RD=rzl; goto info_ill2;
case LD_E_L:	RE=rzl; goto info_ill2;
case LD_A_L:	ra=rzl; goto info_ill2;
info_ill2:		Z80_INFO_ILLEGAL(cc-8,pc-2); LOOP;
case LD_H_A:	rzh=ra; goto info_ill2;
case LD_L_A:	rzl=ra; goto info_ill2;
case LD_H_N:	GET_N(rzh); goto info_ill2;
case LD_L_N:	GET_N(rzl); goto info_ill2;
case DEC_H:		M_DEC(rzh); goto info_ill2;
case DEC_L:		M_DEC(rzl); goto info_ill2;
case INC_H:		M_INC(rzh); goto info_ill2;
case INC_L:		M_INC(rzl); goto info_ill2;
case ADD_H:		M_ADD(rzh); goto info_ill2;
case ADD_L:		M_ADD(rzl); goto info_ill2;
case SUB_H:		M_SUB(rzh); goto info_ill2;
case SUB_L:		M_SUB(rzl); goto info_ill2;
case ADC_H:		M_ADC(rzh); goto info_ill2;
case ADC_L:		M_ADC(rzl); goto info_ill2;
case SBC_H:		M_SBC(rzh); goto info_ill2;
case SBC_L:		M_SBC(rzl); goto info_ill2;
case CP_H:		M_CP(rzh); goto info_ill2;
case CP_L:		M_CP(rzl); goto info_ill2;
case AND_H:		M_AND(rzh); goto info_ill2;
case AND_L:		M_AND(rzl); goto info_ill2;
case OR_H:		M_OR(rzh); goto info_ill2;
case OR_L:		M_OR(rzl); goto info_ill2;
case XOR_H:		M_XOR(rzh); goto info_ill2;
case XOR_L:		M_XOR(rzl); goto info_ill2;
case LD_H_H:	rzh=rzh; goto info_ill2;		// weird
case LD_L_H:	rzl=rzh; goto info_ill2;		// weird
case LD_H_L:	rzh=rzl; goto info_ill2;		// weird
case LD_L_L:	rzl=rzl; goto info_ill2;		// weird




case JP_HL:				pc=rz;					LOOP;		// 4+ 4 T
case LD_SP_HL:	cc+=2;	SP=rz;					LOOP;		// 4+ 6 T
case DEC_HL:   	cc+=2;	rz--;					LOOP;		// 4+ 6 T
case INC_HL:	cc+=2;	rz++;					LOOP;		// 4+ 6 T

case ADD_HL_BC:	cc+=7; M_ADDW(rz,BC);			LOOP;		// 4+ pc:4 +11
case ADD_HL_DE:	cc+=7; M_ADDW(rz,DE);			LOOP;
case ADD_HL_HL:	cc+=7; M_ADDW(rz,rz);			LOOP;
case ADD_HL_SP:	cc+=7; M_ADDW(rz,SP);			LOOP;

case PUSH_HL:	cc+=1;	PUSH(rzh); PUSH(rzl);	LOOP;		// 4+ pc:5, sp-1:3, sp-2:3
case POP_HL:			POP(rzl);  POP(rzh);	LOOP;		// 4+ pc:4, sp:3, sp+1:3

case LD_HL_NN:			GET_NN(rz);				LOOP;		// 4+ pc:4, pc+1:3, pc+2:3

case LD_xNN_HL:	GET_NN(w); POKE(w,rzl); POKE_AND_LOOP(w+1,rzh);	// 4+ pc:4, pc+1:3, pc+2:3, nn:3, nn+1:3
case LD_HL_xNN:	GET_NN(w); PEEK(rzl,w); PEEK(rzh,w+1);	LOOP;	// 4+ pc:4, pc+1:3, pc+2:3, nn:3, nn+1:3

case EX_HL_xSP:	w=rz;									// pc:4, pc+1:4, sp:3, sp+1:3,1, sp+1:3, sp:3,2x1		((total:4+19; seq.: m1,m1,r,r,w,w))
				PEEK(rzl,SP); PEEK(rzh,SP+1); SKIP_1CC(SP+1);
				POKE(SP+1,w>>8); POKE(SP,w);  SKIP_2X1CC(SP);		 // kio tested 2005-01-15
				Z80_INFO_EX_HL_xSP;
				LOOP;





// ######## opcodes with dis ############################

{ /* signed */ int8 dis;
case LD_xHL_B:	c=RB; goto ld_x_c;			// pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
case LD_xHL_C:	c=RC; goto ld_x_c;
case LD_xHL_D:	c=RD; goto ld_x_c;
case LD_xHL_E:	c=RE; goto ld_x_c;
case LD_xHL_H:	c=RH; goto ld_x_c;
case LD_xHL_L:	c=RL; goto ld_x_c;
case LD_xHL_A:	c=ra; goto ld_x_c;
	   ld_x_c:	GET_N(dis); SKIP_5X1CC(pc-1); POKE_AND_LOOP(rz+dis,c);

{ uint8* p;
case LD_B_xHL:	p=&RB; goto ld_p_x;			// pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
case LD_C_xHL:	p=&RC; goto ld_p_x;
case LD_D_xHL:	p=&RD; goto ld_p_x;
case LD_E_xHL:	p=&RE; goto ld_p_x;
case LD_H_xHL:	p=&RH; goto ld_p_x;
case LD_L_xHL:	p=&RL; goto ld_p_x;
		ld_p_x:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(*p,w);	LOOP;
}
case LD_A_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(ra,w);	LOOP;

case LD_xHL_N:	GET_N(dis); GET_N(c); SKIP_2X1CC(pc-1);					// timing:	pc:4, pc+1:4, pc+2:3, pc+3:3,2x1, ix+n:3		korr kio 2005-01-14/15
				POKE_AND_LOOP(rz+dis,c);
case DEC_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis;					// timing:	pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3,1, ix+n:3
				PEEK(c,w);  SKIP_1CC(w); M_DEC(c); POKE_AND_LOOP(w,c);
case INC_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis;
				PEEK(c,w);  SKIP_1CC(w); M_INC(c); POKE_AND_LOOP(w,c);

case ADD_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_ADD(c);	LOOP;	// pc:4, pc+1:4, pc+2:3, pc+2:1x5, ix+n:3
case SUB_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_SUB(c);	LOOP;
case ADC_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_ADC(c);	LOOP;
case SBC_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_SBC(c);	LOOP;
case CP_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_CP(c);		LOOP;
case AND_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_AND(c);	LOOP;
case OR_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_OR(c);		LOOP;
case XOR_xHL:	GET_N(dis); SKIP_5X1CC(pc-1); w=rz+dis; PEEK(c,w); M_XOR(c);	LOOP;
}	// dis

case PFX_CB:
//	Within an 8-instruction block, every illegal DD CB instruction works as
//	the official one, but also copies the result to the specified register.
//	((The information about the inofficial CB instructions was given to   ))
//	((Gerton Lunter by Arnt Gulbrandsen, and originated from David Librik.))
	{	uint8 o; /*signed*/ int8 dis;



	// SHIFT, SET, and RES: 23 T cycles; BIT: 20 T cycles

	GET_N(dis);	w = rz+dis;		// target address
	GET_XYCB_OP(o);				// xycb opcode. does not increment r register!
	PEEK(c,w); SKIP_1CC(w);		// target

	switch (o>>3)				// instruction
	{
	case BIT0_xHL>>3: M_BIT(0x01,c); 	break;
	case BIT1_xHL>>3: M_BIT(0x02,c); 	break;	// timing:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1
	case BIT2_xHL>>3: M_BIT(0x04,c); 	break;
	case BIT3_xHL>>3: M_BIT(0x08,c); 	break;
	case BIT4_xHL>>3: M_BIT(0x10,c); 	break;
	case BIT5_xHL>>3: M_BIT(0x20,c); 	break;
	case BIT6_xHL>>3: M_BIT(0x40,c); 	break;
	case BIT7_xHL>>3: M_BIT(0x80,c); 	break;

	case RLC_xHL>>3:  M_RLC(c);	 	break;
	case RRC_xHL>>3:  M_RRC(c);	 	break;		// timing:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1, ix+n:3
	case RL_xHL>>3:	  M_RL(c); 	 	break;		// note: ergebnis wird noch via POKE_AND_LOOP geschrieben
	case RR_xHL>>3:	  M_RR(c); 	 	break;
	case SLA_xHL>>3:  M_SLA(c);	 	break;
	case SRA_xHL>>3:  M_SRA(c);	 	break;
	case SLL_xHL>>3:  M_SLL(c);	 	break;
	case SRL_xHL>>3:  M_SRL(c);	 	break;

	case RES0_xHL>>3: c &= ~0x01; 	break;		// timing:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1, ix+n:3
	case RES1_xHL>>3: c &= ~0x02; 	break;		// note: ergebnis wird noch via POKE_AND_LOOP geschrieben
	case RES2_xHL>>3: c &= ~0x04; 	break;
	case RES3_xHL>>3: c &= ~0x08; 	break;
	case RES4_xHL>>3: c &= ~0x10; 	break;
	case RES5_xHL>>3: c &= ~0x20; 	break;
	case RES6_xHL>>3: c &= ~0x40; 	break;
	case RES7_xHL>>3: c &= ~0x80; 	break;

	case SET0_xHL>>3: c |= 0x01;  	break;		// timing:	pc:4, pc+1:4, pc+2:3, pc+3:3,2, ix+n:3,1, ix+n:3
	case SET1_xHL>>3: c |= 0x02;  	break;		// note: ergebnis wird noch via POKE_AND_LOOP geschrieben
	case SET2_xHL>>3: c |= 0x04;  	break;
	case SET3_xHL>>3: c |= 0x08;  	break;
	case SET4_xHL>>3: c |= 0x10;  	break;
	case SET5_xHL>>3: c |= 0x20;  	break;
	case SET6_xHL>>3: c |= 0x40;  	break;
	case SET7_xHL>>3: c |= 0x80;  	break;
	};

	switch(o&0x07)	// copy result to register (illegal opcodes only)
	{
	case 0:	RB=c; 	break;
	case 1:	RC=c; 	break;
	case 2:	RD=c; 	break;
	case 3:	RE=c; 	break;
	case 4:	RH=c; 	break;
	case 5:	RL=c; 	break;
	case 6:	if ( (o&0xc0)==0x40 ) LOOP;				// BIT
			if ((o>>3)==(SLL_xHL>>3)) { Z80_INFO_ILLEGAL(cc-20,pc-4); }	// SLL
			POKE_AND_LOOP(w,c);						// SET, RES or SHIFT
	case 7:	ra=c; 	break;
	};

	Z80_INFO_ILLEGAL(cc-20,pc-4);
	if ((o&0xc0)==0x40) LOOP;						// BIT
	POKE_AND_LOOP(w,c);								// SET, RES or SHIFT
	};


default:			// ix/iy prefix has no effect on operation:
					// => prefix worked like a NOP:
	Z80_INFO_ILLEGAL(cc-8,pc-2);	// weird illegal
	cc-=4;							// undo cc+=4
	r--;							// undo r++
	pc--;							// undo pc++	=> execute instruction without prefix
	goto loop_ei;	// no interrupt between prefix and next opcode possible  (source: z80-documented.pdf)
};




#ifndef Z80_H
}
#endif

















