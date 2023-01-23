// Copyright (c) 1996 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	Z80 Emulator
	based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	CB instruction handlers


note:
this is the 'compact version' which fit's in a 32k code segment for 68020 version
and which is faster than a case construct with 256 distinct handlers on a PowerPC,
probably due to more cache hits. Therefore i have removed the version with the 256
handlers on 19.Nov.1995. KIO !


timing:
	all register operations take 8 T cycles:  pc:4, pc+1:4
	bit test operations (hl):				  pc:4, pc+1:4, hl:3,1
	shift and bit set/clear (hl):			  pc:4, pc+1:4, hl:3,1, hl:3
*/


#ifndef Z80_H
void xxx(){		// keep the syntax checker happy
#endif


GET_CB_OP(w);						// fetch opcode


// read source: b,c,d,e,h,l,(hl),a
switch ( w&0x07 )
{
case 0:			c=RB;				break;
case 1:			c=RC;				break;
case 2:			c=RD;				break;
case 3:			c=RE;				break;
case 4:			c=RH;				break;
case 5:			c=RL;				break;
case 6:			PEEK(c,HL); SKIP_1CC(HL); break;
case 7:			c=ra;				break;
};

// perform operation: shift/bit/res/set
switch ( w>>3 )
{
case RLC_B>>3:	M_RLC(c);			break;
case RRC_B>>3:	M_RRC(c);			break;
case RL_B>>3:	M_RL(c);			break;
case RR_B>>3:	M_RR(c);			break;
case SLA_B>>3:	M_SLA(c);			break;
case SRA_B>>3:	M_SRA(c);			break;
case SLL_B>>3:	Z80_INFO_ILLEGAL(cc-8,pc-2);	M_SLL(c); break;
case SRL_B>>3:	M_SRL(c);			break;

case BIT0_B>>3:	M_BIT(0x01,c);		LOOP;	// bit tests have no store-back
case BIT1_B>>3:	M_BIT(0x02,c);		LOOP;
case BIT2_B>>3:	M_BIT(0x04,c);		LOOP;
case BIT3_B>>3:	M_BIT(0x08,c);		LOOP;
case BIT4_B>>3:	M_BIT(0x10,c);		LOOP;
case BIT5_B>>3:	M_BIT(0x20,c);		LOOP;
case BIT6_B>>3:	M_BIT(0x40,c);		LOOP;
case BIT7_B>>3:	M_BIT(0x80,c);		LOOP;

case RES0_B>>3:	c&=~0x01;			break;
case RES1_B>>3:	c&=~0x02;			break;
case RES2_B>>3:	c&=~0x04;			break;
case RES3_B>>3:	c&=~0x08;			break;
case RES4_B>>3:	c&=~0x10;			break;
case RES5_B>>3:	c&=~0x20;			break;
case RES6_B>>3:	c&=~0x40;			break;
case RES7_B>>3:	c&=~0x80;			break;

case SET0_B>>3:	c|=0x01;			break;
case SET1_B>>3:	c|=0x02;			break;
case SET2_B>>3:	c|=0x04;			break;
case SET3_B>>3:	c|=0x08;			break;
case SET4_B>>3:	c|=0x10;			break;
case SET5_B>>3:	c|=0x20;			break;
case SET6_B>>3:	c|=0x40;			break;
case SET7_B>>3:	c|=0x80;			break;
};

// store back result:
switch ( w&0x07 )
{
case 0:			RB=c;				LOOP;
case 1:			RC=c;				LOOP;
case 2:			RD=c;				LOOP;
case 3:			RE=c;				LOOP;
case 4:			RH=c;				LOOP;
case 5:			RL=c;				LOOP;
case 6:			POKE_AND_LOOP(HL,c);
case 7:			ra=c;				LOOP;
};



#ifndef Z80_H
}
#endif





