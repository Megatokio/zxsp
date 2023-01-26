// Copyright (c) 1996 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	Z80 Emulator
	originally based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	unprefixed instruction handlers
*/


GET_INSTR(c);

switch ( c )
{

// ########	4 T cycle Instructions #########################

case LD_B_B:	RB=RB;	LOOP;				// 4 T
case LD_C_B:	RC=RB;	LOOP;				// 4 T
case LD_D_B:	RD=RB;	LOOP;				// 4 T
case LD_E_B:	RE=RB;	LOOP;				// 4 T
case LD_H_B:	RH=RB;	LOOP;				// 4 T
case LD_L_B:	RL=RB;	LOOP;				// 4 T
case LD_A_B:	ra=RB;	LOOP;				// 4 T

case LD_B_C:	RB=RC;	LOOP;				// 4 T
case LD_C_C:	RC=RC;	LOOP;				// 4 T
case LD_D_C:	RD=RC;	LOOP;				// 4 T
case LD_E_C:	RE=RC;	LOOP;				// 4 T
case LD_H_C:	RH=RC;	LOOP;				// 4 T
case LD_L_C:	RL=RC;	LOOP;				// 4 T
case LD_A_C:	ra=RC;	LOOP;				// 4 T

case LD_B_D:	RB=RD;	LOOP;				// 4 T
case LD_C_D:	RC=RD;	LOOP;				// 4 T
case LD_D_D:	RD=RD;	LOOP;				// 4 T
case LD_E_D:	RE=RD;	LOOP;				// 4 T
case LD_H_D:	RH=RD;	LOOP;				// 4 T
case LD_L_D:	RL=RD;	LOOP;				// 4 T
case LD_A_D:	ra=RD;	LOOP;				// 4 T

case LD_B_E:	RB=RE;	LOOP;				// 4 T
case LD_C_E:	RC=RE;	LOOP;				// 4 T
case LD_D_E:	RD=RE;	LOOP;				// 4 T
case LD_E_E:	RE=RE;	LOOP;				// 4 T
case LD_H_E:	RH=RE;	LOOP;				// 4 T
case LD_L_E:	RL=RE;	LOOP;				// 4 T
case LD_A_E:	ra=RE;	LOOP;				// 4 T

case LD_B_H:	RB=RH;	LOOP;				// 4 T
case LD_C_H:	RC=RH;	LOOP;				// 4 T
case LD_D_H:	RD=RH;	LOOP;				// 4 T
case LD_E_H:	RE=RH;	LOOP;				// 4 T
case LD_H_H:	RH=RH;	LOOP;				// 4 T
case LD_L_H:	RL=RH;	LOOP;				// 4 T
case LD_A_H:	ra=RH;	LOOP;				// 4 T

case LD_B_L:	RB=RL;	LOOP;				// 4 T
case LD_C_L:	RC=RL;	LOOP;				// 4 T
case LD_D_L:	RD=RL;	LOOP;				// 4 T
case LD_E_L:	RE=RL;	LOOP;				// 4 T
case LD_H_L:	RH=RL;	LOOP;				// 4 T
case LD_L_L:	RL=RL;	LOOP;				// 4 T
case LD_A_L:	ra=RL;	LOOP;				// 4 T

case LD_B_A:	RB=ra;	LOOP;				// 4 T
case LD_C_A:	RC=ra;	LOOP;				// 4 T
case LD_D_A:	RD=ra;	LOOP;				// 4 T
case LD_E_A:	RE=ra;	LOOP;				// 4 T
case LD_H_A:	RH=ra;	LOOP;				// 4 T
case LD_L_A:	RL=ra;	LOOP;				// 4 T
case LD_A_A:  /*ra=ra;*/LOOP;				// 4 T

case ADD_B:		M_ADD(RB);	LOOP;			// 4 T
case ADD_C:		M_ADD(RC);	LOOP;			// 4 T
case ADD_D:		M_ADD(RD);	LOOP;			// 4 T
case ADD_E:		M_ADD(RE);	LOOP;			// 4 T
case ADD_H:		M_ADD(RH);	LOOP;			// 4 T
case ADD_L:		M_ADD(RL);	LOOP;			// 4 T
case ADD_A:		M_ADD(ra);	LOOP;			// 4 T

case SUB_B:		M_SUB(RB);	LOOP;			// 4 T
case SUB_C:		M_SUB(RC);	LOOP;			// 4 T
case SUB_D:		M_SUB(RD);	LOOP;			// 4 T
case SUB_E:		M_SUB(RE);	LOOP;			// 4 T
case SUB_H:		M_SUB(RH);	LOOP;			// 4 T
case SUB_L:		M_SUB(RL);	LOOP;			// 4 T
case SUB_A:		M_SUB(ra);	LOOP;			// 4 T

case ADC_B:		M_ADC(RB);	LOOP;			// 4 T
case ADC_C:		M_ADC(RC);	LOOP;			// 4 T
case ADC_D:		M_ADC(RD);	LOOP;			// 4 T
case ADC_E:		M_ADC(RE);	LOOP;			// 4 T
case ADC_H:		M_ADC(RH);	LOOP;			// 4 T
case ADC_L:		M_ADC(RL);	LOOP;			// 4 T
case ADC_A:		M_ADC(ra);	LOOP;			// 4 T

case SBC_B:		M_SBC(RB);	LOOP;			// 4 T
case SBC_C:		M_SBC(RC);	LOOP;			// 4 T
case SBC_D:		M_SBC(RD);	LOOP;			// 4 T
case SBC_E:		M_SBC(RE);	LOOP;			// 4 T
case SBC_H:		M_SBC(RH);	LOOP;			// 4 T
case SBC_L:		M_SBC(RL);	LOOP;			// 4 T
case SBC_A:		M_SBC(ra);	LOOP;			// 4 T

case CP_B:		M_CP(RB); LOOP;				// 4 T
case CP_C:		M_CP(RC); LOOP;				// 4 T
case CP_D:		M_CP(RD); LOOP;				// 4 T
case CP_E:		M_CP(RE); LOOP;				// 4 T
case CP_H:		M_CP(RH); LOOP;				// 4 T
case CP_L:		M_CP(RL); LOOP;				// 4 T
case CP_A:		M_CP(ra); LOOP;				// 4 T

case AND_B:		M_AND(RB);LOOP;				// 4 T
case AND_C:		M_AND(RC);LOOP;				// 4 T
case AND_D:		M_AND(RD);LOOP;				// 4 T
case AND_E:		M_AND(RE);LOOP;				// 4 T
case AND_H:		M_AND(RH);LOOP;				// 4 T
case AND_L:		M_AND(RL);LOOP;				// 4 T
case AND_A:		M_AND(ra);LOOP;				// 4 T

case OR_B:		M_OR(RB);LOOP;				// 4 T
case OR_C:		M_OR(RC);LOOP;				// 4 T
case OR_D:		M_OR(RD);LOOP;				// 4 T
case OR_E:		M_OR(RE);LOOP;				// 4 T
case OR_H:		M_OR(RH);LOOP;				// 4 T
case OR_L:		M_OR(RL);LOOP;				// 4 T
case OR_A:		M_OR(ra);LOOP;				// 4 T

case XOR_B:		M_XOR(RB);LOOP;				// 4 T
case XOR_C:		M_XOR(RC);LOOP;				// 4 T
case XOR_D:		M_XOR(RD);LOOP;				// 4 T
case XOR_E:		M_XOR(RE);LOOP;				// 4 T
case XOR_H:		M_XOR(RH);LOOP;				// 4 T
case XOR_L:		M_XOR(RL);LOOP;				// 4 T
case XOR_A:		M_XOR(ra);LOOP;				// 4 T

case DEC_B:		M_DEC(RB);LOOP;				// 4 T
case DEC_C:		M_DEC(RC);LOOP;				// 4 T
case DEC_D:		M_DEC(RD);LOOP;				// 4 T
case DEC_E:		M_DEC(RE);LOOP;				// 4 T
case DEC_H:		M_DEC(RH);LOOP;				// 4 T
case DEC_L:		M_DEC(RL);LOOP;				// 4 T
case DEC_A:		M_DEC(ra);LOOP;				// 4 T

case INC_B:		M_INC(RB);LOOP;				// 4 T
case INC_C:		M_INC(RC);LOOP;				// 4 T
case INC_D:		M_INC(RD);LOOP;				// 4 T
case INC_E:		M_INC(RE);LOOP;				// 4 T
case INC_H:		M_INC(RH);LOOP;				// 4 T
case INC_L:		M_INC(RL);LOOP;				// 4 T
case INC_A:		M_INC(ra);LOOP;				// 4 T

case JP_HL:		pc = HL;  LOOP;				// 4 T

case EX_DE_HL:	w=DE;DE=HL;HL=w;			// 4 T
				LOOP;
case EX_AF_AF:	c=ra;ra=RA2;RA2=c;			// 4 T
				c=rf;rf=RF2;RF2=c;
				LOOP;
case EXX:		w=BC;BC=BC2;BC2=w;			// 4 T
				w=DE;DE=DE2;DE2=w;
				w=HL;HL=HL2;HL2=w;
				LOOP;

case HALT:		pc--; Z80_INFO_HALT; LOOP;	// 4 T  ((executes NOPs))
case NOP:		LOOP;						// 4 T
case DI:		IFF1=IFF2=disabled;	LOOP;	// 4 T
case EI:	//	if (IFF1==enabled)	LOOP;	// 4 T		lt. z80-documented.pdf: nach EI niemals Irpt Ackn
				IFF1=IFF2=enabled;
				Z80_INFO_EI;
				if (cc >= cc_nmi) LOOP;		// NMI can start after EI
				goto loop_ei;	// der nächste befehl wird auf jeden fall noch ausgeführt =>
								// kein sprung via LOOP nach nxtcmd, weil dort evtl. wg. cc>=ccx Run() verlassen würde
								// und beim Wiederaufruf zuerst die Interrupts geprüft werden.

case SCF:		rf|=C_FLAG; rf&=~(N_FLAG+H_FLAG);	LOOP;		// 4 T
case CCF:		rf^=C_FLAG; rf&=~N_FLAG;			LOOP;		// 4 T
case CPL:		ra=~ra; rf|=N_FLAG+H_FLAG;			LOOP;		// 4 T

case RLCA:		ra = (ra<<1) + (ra>>7);				// 4 T
				rf = (rf&~(C_FLAG+N_FLAG+H_FLAG)) + (ra&C_FLAG);
				LOOP;
case RRCA:		ra = (ra>>1) + (ra<<7);				// 4 T
				rf = (rf&~(C_FLAG+N_FLAG+H_FLAG)) + (ra>>7);
				LOOP;
case RLA:		c  = ra>>7;							// 4 T
				ra = (ra<<1) + (rf&C_FLAG);
				rf = (rf&~(C_FLAG+N_FLAG+H_FLAG)) + c;
				LOOP;
case RRA:		c  = ra&C_FLAG;						// 4 T
				ra = (ra>>1) + (rf<<7);
				rf = (rf&~(C_FLAG+N_FLAG+H_FLAG)) + c;
				LOOP;

case DAA:		if (rf&N_FLAG)						// 4 T
				{	// previous instruction was SUB
					if (rf&H_FLAG) 		ra -= 0x06;
					if (rf&C_FLAG) 		ra -= 0x60;
				}
				else
				{	// previous instruction was ADD
					if ((ra&0x0F)>0x09)	rf |= H_FLAG;
					if (rf&H_FLAG) 		ra += 0x06;
					if (ra>0x99) 		rf |= C_FLAG;
					if (rf&C_FLAG)		ra += 0x60;
				};
				rf &= C_FLAG+N_FLAG;
				rf |= zlog_flags[ra];
				LOOP;

// ========	END OF 4 T INSTRUCTIONS ========================




// ########	Other no-memory-access Instructions #########################

case DEC_BC:	cc+=2; BC--;		LOOP;		// T 6
case DEC_DE:	cc+=2; DE--;		LOOP;		// T 6
case DEC_HL:	cc+=2; HL--;		LOOP;		// T 6
case DEC_SP:	cc+=2; SP--;		LOOP;		// T 6
case INC_BC:	cc+=2; BC++;		LOOP;		// T 6
case INC_DE:	cc+=2; DE++;		LOOP;		// T 6
case INC_HL:	cc+=2; HL++;		LOOP;		// T 6
case INC_SP:	cc+=2; SP++;		LOOP;		// T 6

case LD_SP_HL:	cc+=2; SP = HL;	LOOP;		// T 6

case ADD_HL_BC:	cc+=7; M_ADDW(HL,BC); LOOP;	// T 11
case ADD_HL_DE:	cc+=7; M_ADDW(HL,DE); LOOP;	// T 11
case ADD_HL_HL:	cc+=7; M_ADDW(HL,HL); LOOP;	// T 11
case ADD_HL_SP:	cc+=7; M_ADDW(HL,SP); LOOP;	// T 11




// ########	Read-only-from-memory Instructions #########################


{ uint8* p;
case LD_B_xHL:	p=&RB; goto ld_xhl;
case LD_C_xHL:	p=&RC; goto ld_xhl;
case LD_D_xHL:	p=&RD; goto ld_xhl;
case LD_E_xHL:	p=&RE; goto ld_xhl;
case LD_H_xHL:	p=&RH; goto ld_xhl;
case LD_L_xHL:	p=&RL; goto ld_xhl;
		ld_xhl:	PEEK(*p,HL); LOOP;		// timing: pc:4, hl:3
}


{ uint8* p;
case LD_B_N:	p=&RB; goto ld_n;
case LD_C_N:	p=&RC; goto ld_n;
case LD_D_N:	p=&RD; goto ld_n;
case LD_E_N:	p=&RE; goto ld_n;
case LD_H_N:	p=&RH; goto ld_n;
case LD_L_N:	p=&RL; goto ld_n;
		ld_n:	GET_N(*p); LOOP;		// timing: pc:4, pc+1:3
}
case LD_A_N:	GET_N(ra); LOOP;		// timing: pc:4, pc+1:3


case ADD_xHL:		PEEK(c,HL);   M_ADD(c); LOOP;		// timing: pc:4, hl:3
case SUB_xHL:		PEEK(c,HL);   M_SUB(c); LOOP;		// timing: pc:4, hl:3
case ADC_xHL:		PEEK(c,HL);   M_ADC(c); LOOP;		// timing: pc:4, hl:3
case SBC_xHL:		PEEK(c,HL);   M_SBC(c); LOOP;		// timing: pc:4, hl:3
case CP_xHL:		PEEK(c,HL);   M_CP(c);  LOOP;		// timing: pc:4, hl:3
case OR_xHL:		PEEK(c,HL);   M_OR(c);  LOOP;		// timing: pc:4, hl:3
case XOR_xHL:		PEEK(c,HL);   M_XOR(c); LOOP;		// timing: pc:4, hl:3
case AND_xHL:		PEEK(c,HL);   M_AND(c); LOOP;		// timing: pc:4, hl:3


case ADD_N:		GET_N(c);   M_ADD(c); LOOP;		// timing: pc:4, pc+1:3
case ADC_N:		GET_N(c);   M_ADC(c); LOOP;		// timing: pc:4, pc+1:3
case SUB_N:		GET_N(c);   M_SUB(c); LOOP;		// timing: pc:4, pc+1:3
case SBC_N:		GET_N(c);   M_SBC(c); LOOP;		// timing: pc:4, pc+1:3
case CP_N:		GET_N(c);   M_CP(c);  LOOP;		// timing: pc:4, pc+1:3
case OR_N:		GET_N(c);   M_OR(c);  LOOP;		// timing: pc:4, pc+1:3
case XOR_N:		GET_N(c);   M_XOR(c); LOOP;		// timing: pc:4, pc+1:3
case AND_N:		GET_N(c);   M_AND(c); LOOP;		// timing: pc:4, pc+1:3


case LD_SP_NN: 	rzp=&registers.sp; goto ld_nn;
case LD_BC_NN: 	rzp=&registers.bc; goto ld_nn;
case LD_DE_NN: 	rzp=&registers.de; goto ld_nn;
case LD_HL_NN: 	rzp=&registers.hl; goto ld_nn;
		ld_nn:	GET_N(rzl); GET_N(rzh); LOOP;			// timing: pc:4,pc+1:3,pc+2:3


case JP_NZ:		if (rf&Z_FLAG) goto njp; else goto jp;
case JP_NC:		if (rf&C_FLAG) goto njp; else goto jp;
case JP_PO:		if (rf&P_FLAG) goto njp; else goto jp;
case JP_P:		if (rf&S_FLAG) goto njp; else goto jp;
case JP_C:		if (rf&C_FLAG) goto jp; else goto njp;
case JP_PE:		if (rf&P_FLAG) goto jp; else goto njp;
case JP_M:		if (rf&S_FLAG) goto jp; else goto njp;
case JP_Z:		if (rf&Z_FLAG) goto jp; else goto njp;
		   njp:	SKIP_N(); SKIP_N(); LOOP;							// timing: pc:4,pc+1:3,pc+2:3
case JP:	jp:	GET_NN(w); pc=w; LOOP;								// timing: pc:4,pc+1:3,pc+2:3


case JR:	jr:	GET_N(c); SKIP_5X1CC(pc); pc+=(signed char)c; LOOP;	// timing: pc:4, pc+1:3,5*1
case JR_Z:		if(rf&Z_FLAG) goto jr;								// timing: pc:4, pc+1:3,[5*1]
		   njr:	SKIP_N(); LOOP;
case JR_C:		if(rf&C_FLAG) goto jr; else goto njr;				// timing: pc:4, pc+1:3,[5*1]
case JR_NZ:		if(rf&Z_FLAG) goto njr; else goto jr;				// timing: pc:4, pc+1:3,[5*1]
case JR_NC:		if(rf&C_FLAG) goto njr; else goto jr;				// timing: pc:4, pc+1:3,[5*1]
case DJNZ:		cc+=1; if(--RB) goto jr; else goto njr;				// timing: pc:5, pc+1:3,[5*1]


case RET:  ret:	POP(PCL); POP(PCH); pc=PC;							// timing: pc:4, sp:3, sp+1:3
				Z80_INFO_RET;
				LOOP;

case RET_NZ:	cc+=1; if(rf&Z_FLAG) LOOP; else goto ret;			// timing: pc:5, [sp:3, sp+1:3]
case RET_NC:	cc+=1; if(rf&C_FLAG) LOOP; else goto ret;
case RET_PO:	cc+=1; if(rf&P_FLAG) LOOP; else goto ret;
case RET_P:		cc+=1; if(rf&S_FLAG) LOOP; else goto ret;
case RET_Z:		cc+=1; if(rf&Z_FLAG) goto ret; else LOOP;
case RET_C:		cc+=1; if(rf&C_FLAG) goto ret; else LOOP;
case RET_PE:	cc+=1; if(rf&P_FLAG) goto ret; else LOOP;
case RET_M:		cc+=1; if(rf&S_FLAG) goto ret; else LOOP;


case LD_A_xNN:	GET_NN(w); goto ld_a_xw;	// timing: pc:4, pc+1:3, pc+2:3, nn:3
case LD_A_xBC:	w=BC;      goto ld_a_xw;	// timing: pc:4, bc:3
case LD_A_xDE:	w=DE;	   goto ld_a_xw;	// timing: pc:4, de:3
case LD_A_xHL:	w=HL;	   goto ld_a_xw;	// timing: pc:4, hl:3
	  ld_a_xw:	PEEK(ra,w); LOOP;

case LD_HL_xNN:	GET_NN(w); PEEK(RL,w); PEEK(RH,w+1); LOOP;			// timing: pc:4, pc+1:3, pc+2:3, nn:3, nn+1:3


case POP_BC:	rzp=&registers.bc; goto pop_rr;
case POP_DE:	rzp=&registers.de; goto pop_rr;
case POP_HL:	rzp=&registers.hl; goto pop_rr;
		pop_rr:	POP(rzl); POP(rzh);									// timing: pc:4, sp:3, sp+1:3
				goto pop_af;
case POP_AF:	POP(rf); POP(ra);									// timing: pc:4, sp:3, sp+1:3
		pop_af:	Z80_INFO_POP;
				LOOP;


case OUTA:		GET_N(c); OUTPUT ( ra*256 + c, ra ); LOOP;			// timing: pc:4, pc+1:3, IO
case INA:		GET_N(c); INPUT ( ra*256 + c, ra ); LOOP;			// timing: pc:4, pc+1:3, IO






// ########	Write-to-memory Instructions #####################
case CALL_NC: 	if (rf&C_FLAG) goto nocall; else goto call;			// pc:4, pc+1:3, pc+2:3, [pc+2:1, sp-1:3, sp-2:3]
case CALL_PO:	if (rf&P_FLAG) goto nocall; else goto call;
case CALL_P: 	if (rf&S_FLAG) goto nocall; else goto call;
case CALL_NZ: 	if (rf&Z_FLAG) goto nocall; else goto call;
case CALL_C: 	if (rf&C_FLAG) goto call; else goto nocall;
case CALL_PE:	if (rf&P_FLAG) goto call; else goto nocall;
case CALL_M: 	if (rf&S_FLAG) goto call; else goto nocall;
case CALL_Z: 	if (rf&Z_FLAG) goto call; else goto nocall;
	 nocall:	SKIP_N(); SKIP_N(); LOOP;
case CALL:
	 call:		GET_NN(w); SKIP_1CC(pc-1);							// pc:4, pc+1:3, pc+2:3,1, sp-1:3, sp-2:3
	 rst:		PUSH(pc>>8); PUSH(pc); pc=w; LOOP;

case RST00:		w=0x0000; cc+=1; Z80_INFO_RST00; goto rst;			// pc:5, sp-1:3, sp-2:3
case RST08:  	w=0x0008; cc+=1; Z80_INFO_RST08; goto rst;
case RST10:  	w=0x0010; cc+=1; Z80_INFO_RST10; goto rst;
case RST18:  	w=0x0018; cc+=1; Z80_INFO_RST18; goto rst;
case RST20:  	w=0x0020; cc+=1; Z80_INFO_RST20; goto rst;
case RST28:  	w=0x0028; cc+=1; Z80_INFO_RST28; goto rst;
case RST30:  	w=0x0030; cc+=1; Z80_INFO_RST30; goto rst;
case RST38:  	w=0x0038; cc+=1; Z80_INFO_RST38; goto rst;

case DEC_xHL:	w=HL; PEEK(c,w); SKIP_1CC(w); M_DEC(c); POKE_AND_LOOP(w,c);		// pc:4, hl:3,1, hl:3
case INC_xHL:	w=HL; PEEK(c,w); SKIP_1CC(w); M_INC(c); POKE_AND_LOOP(w,c);		// pc:4, hl:3,1, hl:3

case LD_xHL_B:	POKE_AND_LOOP(HL,RB);						// timing: pc:4, hl:3
case LD_xHL_C:	POKE_AND_LOOP(HL,RC);
case LD_xHL_D:	POKE_AND_LOOP(HL,RD);
case LD_xHL_E:	POKE_AND_LOOP(HL,RE);
case LD_xHL_H:	POKE_AND_LOOP(HL,RH);
case LD_xHL_L:	POKE_AND_LOOP(HL,RL);

case LD_xHL_A:	POKE_AND_LOOP(HL,ra);
case LD_xBC_A:	POKE_AND_LOOP(BC,ra);						// pc:4, bc:3
case LD_xDE_A:	POKE_AND_LOOP(DE,ra);						// pc:4, de:3

case LD_xHL_N:	GET_N(c); POKE_AND_LOOP(HL,c);				// pc:4, pc+1:3, hl:3

case LD_xNN_A:	GET_NN(w); POKE_AND_LOOP(w,ra);				// pc:4, pc+1:3, pc+2:3, nn:3

case LD_xNN_HL:	GET_NN(w); POKE(w,RL); POKE_AND_LOOP(w+1,RH);	// pc:4, pc+1:3, pc+2:3, nn:3, nn+1:3

case PUSH_BC:	w=BC; goto push_w;
case PUSH_DE:	w=DE; goto push_w;
case PUSH_HL:	w=HL; goto push_w;
		push_w:	cc+=1; PUSH(wh); PUSH(wl);	LOOP;			// pc:5, sp-1:3, sp-2:3
case PUSH_AF:	cc+=1; PUSH(ra); PUSH(rf);	LOOP;			// pc:5, sp-1:3, sp-2:3

case EX_HL_xSP:	w=HL;										// pc:4, sp:3, sp+1:3,1, sp+1:3, sp:3,2x1		kio tested 2005-01-10/14/15
				PEEK(RL,SP); PEEK(RH,SP+1); SKIP_1CC(SP+1);
				POKE(SP+1,wh); POKE(SP,wl); SKIP_2X1CC(SP);
				Z80_INFO_EX_HL_xSP;
				LOOP;



// ----	PREFIX COMMANDS ---------------------------------------------

case PFX_IY:	rzp = &registers.iy; goto XY;	// 4 T
case PFX_IX:	rzp = &registers.ix; XY:		// 4 T
#include "Z80codesXY.h"
case PFX_CB:									// 4 T: Timing berücksichtigt im CB-Dispatcher
#include "Z80codesCB.h"
case PFX_ED:									// 4 T: Timing berücksichtigt im ED-Dispatcher
#include "Z80codesED.h"


default:	break;
};







