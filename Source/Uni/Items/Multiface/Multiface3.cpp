// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Multiface3.h"
#include "Machine.h"


/*    WoS:
		#define P_MF3_IN                        0x3f
		#define P_MF3_OUT                       0xbf
		#define P_MF3_BUTTON                    0x3f
		#define P_MF3_P7FFD                     0x7f3f
		#define P_MF3_P1FFD                     0x1f3f

The MF3 contains
  8k ROM			note: actually 16k supported
  8k RAM
  1 PAL16L8
  1 4x4bit register
  4 FF				note: actually 5, one FF for the 16k ROM page selection

Ram, Rom:
	RAM.CE = !( !FF_PAGE + !MREQ + !A14 + !A15 + A13 )
	ROM.CE = !( !FF_PAGE + !MREQ + !A14 + !A15 + !A13 )

PAL:
4 outputs, all other pins inputs:
	PAL.WPORTxFFD pin17 = !( !IORQ + !WR + %---1.----.11111101 )
	PAL.WPORT     pin18 = !( !IORQ + !WR + %-011.1111 )
	PAL.RPORT     pin19 = !( !IORQ + !RD + %-011.1111 + (A7 | !FF_HIDE) )
	PAL.CONMEM    pin12 = !( !MREQ + !RD + !M1 + %0000.0000.01100110 + FF_NMI + !FF_ALLRAM )

Flipflops:
	IC7A = FF_HIDE		-> MF3 visible/invisible
	IC3A = FF_NMI 		-> NMI pending
	IC9B = FF_ALLRAM 	-> MMU special mode: normal/all_ram
	IC3B = FF_PAGE 		-> MF3 memory paged in/out

	FF_PAGE.SET = !RESET
	FF_PAGE.RES = !( !MREQ + !RD + !M1 + %0000.0000.01100110 + FF_NMI + !FF_ALLRAM )
	FF_PAGE.CLK = !( !IORQ + !RD + %-011.1111 + (A7 | !FF_HIDE) )
				-> liest A7

	FF_NMI.SET = !( !FF_NMI + !Taster )
	FF_NMI.RES = !RESET
	FF_NMI.CLK = PAL.WPORT = !( !IORQ + !WR + %-011.1111 )
				-> liest 0

	FF_HIDE.SET = !RESET
	FF_HIDE.RES = !( !FF_NMI + !Taster )
	FF_HIDE.CLK = !( !IORQ + !WR + %0011.1111 )
				-> liest 1

	FF_ALLRAM.SET = -
	FF_ALLRAM.CLR = !RESET
	FF_ALLRAM.CLK = !( !IORQ + !WR + %0001.----.11111101 )
				-> liest D0 --> Port $1FFD Bit0 = Paging mode: 1 = Ram only

4x4bit register:
	4x4RAM.addr = A13,A14
	4x4RAM.WR = !( !IORQ + !WR + %0--1.----.11111101 )
	4x4RAM.RD = !( !IORQ + !RD +          %-011.1111 + !FF_HIDE )

NMI:
	NMI = !( FF_NMI && !FF_ALLRAM )


CONCLUSIO:
	• setting the +3 MMU to all_ram disables the MF3,
	  probably because it is impossible to disable internal ram paged in at $0000.
	• the MF+3 has the same camouflage FF as the MF128 has.
	• the MF+3 only activates on $0066 if it's button was pressed, as the MF128 did.
	• out to port %0xx1.----.1111.1101 bits 0-3 are stored in a register,
	  thereof out $7FFD and out $1FFD are useful.
	  a program may fool the MF+3 by using non-standard out addresses for port $7FFD.
	  info in port $1FFD: probably only bit 3 is used: disk motor state.
		other bits are useless and paged-in rom can be deduced by probing, as the MF128 did.
	  info in port $7FFD: probably for bit3 only: screen displayed
		selected ram page at $C000 can be deduced by probing, as the MF128 did.

	in  %----.----.-011.1111		4x4RAM.RD	 (if !disabled)
	in  %----.----.-011.1111		FF_PAGE.CLK: page in/out MF+3 memory (page-in = !A7 & enabled)

	out %0--1.----.1111.1101		4x4RAM.WR:   port 1ffD/7FFD monitor
	out %0001.----.1111.1101		FF_ALLRAM:   disables the MF+3
	out %----.----.-011.1111		FF_NMI.CLK:  disable NMI patch
	out %----.----.0011.1111		FF_HIDE.CLK: hide the MF+3
*/

static cstr i_addr = "----.----.-011.1111";
static cstr o_addr = "----.----.--11.11-1";


Multiface3::Multiface3(Machine* m) :
	Multiface(m, isa_Multiface3, "Roms/mf3.rom", o_addr, i_addr), mf_enabled(), all_ram()
{}


void Multiface3::powerOn(/*t=0*/ int32 cc)
{
	assert(machine->rom.count() == 0x10000);

	Multiface::powerOn(cc);
	mf_enabled = no;
	all_ram	   = no;
	// register4x4[] has no reset input

	machine->rom[0x0066] |= cpu_patch;
	machine->rom[0x4066] |= cpu_patch;
	machine->rom[0x8066] |= cpu_patch;
	machine->rom[0xC066] |= cpu_patch;
}


void Multiface3::reset(Time t, int32 cc)
{
	Multiface::reset(t, cc);
	mf_enabled = no;
	all_ram	   = no;
	// register4x4[] has no reset input
}


/*	Input at registered address: %-0111111
	page ram+rom if bit7=0
	read 4x4bit register
*/
void Multiface3::input(Time, int32, uint16 addr, uint8& byte, uint8& mask)
{
	assert((addr & 0b01111111) == 0b00111111);

	if (mf_enabled)
	{
		if (addr & 0x80)
		{
			if (paged_in) page_out();
		}
		else
		{
			if (!paged_in) page_in();
		}

		byte &= register4x4[(addr >> 13) & 3] | 0xf0;
		mask |= 0x0F;
	}
	else
	{
		if (paged_in) page_out();
	}
}


/*	Output:
	store port $1FFD and $7FFD bits
	set all_ram flag
	re-engage nmi button
*/
void Multiface3::output(Time, int32, uint16 addr, uint8 byte)
{
	if ((addr & 0x90FF) == 0x10fd) // Mmu address: $1FFD or $7FFD
	{
		if ((addr & 0x6000) == 0) all_ram = byte & 1;
		register4x4[(addr >> 13) & 3] = byte;
		return;
	}

	if ((addr & 0x7F) == 0x3F) // enable NMI button, if A7=0 deactivate/hide MF128
	{
		nmi_pending = no;
		if ((addr & 0xFF) == 0x3F) mf_enabled = no;
	}
}


/*	handle rom patch
	test if address = 0x0066 and pages in MF3 memory
		except if no nmi_pending, then s.o. else has pressed nmi
		except if all_ram, then nmi is disabled because MF3 can't page out internal ram
	rearside ROMCS is ignored by the MF3 (or no rear-side connector present)
	note: executing address 0x0066 does not disable the IF3 NMI button
		this is done by the press of the IF3 NMI button itself
		the MF3 can still generate an NMI if this NMI was generated by some other device.
		therefore disabling the button is done in pushNmiButton(), not here.
	returns new opcode
*/
uint8 Multiface3::handleRomPatch(uint16 pc, uint8 o)
{
	if (pc != 0x0066 || !nmi_pending) // not NMI address or NMI not triggered by me
		return prev()->handleRomPatch(pc, o);

	assert(mf_enabled); // nmi_pending should imply mf_enabled

	if (all_ram) return o;
	if (!paged_in) page_in();
	return rom[pc];
}

void Multiface3::triggerNmi()
{
	if (nmi_pending || all_ram) return;

	nmi_pending = yes;
	mf_enabled	= yes;
	machine->cpu->triggerNmi();
}
