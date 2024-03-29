// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface128.h"
#include "Items/Item.h"
#include "Machine.h"
#include "Memory.h"


/*
WoS:
	P_MF128_IN		0xbf  %10111111  use IN A,(191) to page IN		Manual
	P_MF128_IN_V2	0x9f  %10011111  Disciple version				WoS
	P_MF128_OUT		0x3f  %00111111  and IN A,(63) to page OUT		Manual

Multiface 128 acc. to v36 circuit as reverse engineered by velesoft:

	8k ROM
	8k RAM

	rear-side ROMCS ist nicht angeschlossen!
	Wenn das MF-Ram+Rom ausgeblendet ist, ist das Ram auch nicht mehr beschreibbar

	camouflage FF:
		out %0011-1-- or reset: MF128 deactivated / invisible
		nmi					  : MF128 actived / visible

		if invisible:
			in(%1011-1--) does not page in MF128 memory and does not return the video ram bit in D7
						  it even pages out the MF128 memory if it was paged in
		else:
			in(%X011-1--) pages in|out MF128 memory and returns the video ram bit in D7
						  note: das videoram FF wird von reset nicht initialisiert

	out %-011-1-- or reset: enable NMI button

	out %0---.----.----.--0- speichert D3 (das Videoram-Bit)

	reset:
		sets the camouflage FF to "invisible/deactivated"
		schaltet den NMI-Taster scharf
		blendet das MF128 Memory aus

	Taster:
		erzeugt NMI
		deaktiviert den Taster
		sets the camouflage FF to "visible/activated"
		aktiviert die EXE($0066)-Erkennung

	Execute $0066 (NMI-Einsprungadresse)
	decoded: /RD, /MREQ, /M1 und A1-A15 (A0 fehlt)
		wenn der NMI nicht durch den Taster erzeugt wurde:
			passiert nichts
		wenn der NMI durch den Taster des MF128 erzeugt wurde:
			blendet den Speicher des MF128 ein
*/

static cstr o_addr = "----.----.----.----"; // any address!
static cstr i_addr = "----.----.-011.-1--";


Multiface128::Multiface128(Machine* m) :
	Multiface(m, isa_Multiface128, "Roms/mf128.rom", o_addr, i_addr),
	mf_enabled(),
	videopage()
{}


void Multiface128::powerOn(/*t=0*/ int32 cc)
{
	assert(
		machine->rom.count() == 0x8000 || // ZX Spectrum+ 128K
		machine->rom.count() == 0x4000);  // ZX Spectrum

	Multiface::powerOn(cc);
	mf_enabled = no;
	// videopage_bit = 0;		not reset

	for (uint i = 0; i < machine->rom.count(); i += 0x4000)
	{
		machine->rom[i + 0x0066] |= cpu_patch;
		machine->rom[i + 0x0067] |= cpu_patch;
	}
}

void Multiface128::reset(Time t, int32 cc)
{
	Multiface::reset(t, cc);
	mf_enabled = no;
	// videopage_bit = 0;		not reset
}


/*	Input at registered address: %-011-1--
	page ram+rom if bit7=1
*/
void Multiface128::input(Time, int32, uint16 addr, uint8& byte, uint8& mask)
{
	assert((addr & 0b01110100) == 0b00110100);

	if (mf_enabled)
	{
		if (addr & 0x80)
		{
			if (!paged_in) page_in();
		}
		else
		{
			if (paged_in) page_out();
		}

		byte &= videopage | 0x7F;
		mask |= 0x80;
	}
	else
	{
		if (paged_in) page_out();
	}
}


/*	Output:
	store videobit
	nmi-taster wieder scharf schalten
*/
void Multiface128::output(Time, int32, uint16 addr, uint8 byte)
{
	if ((addr & 0x8002) == 0) // Mmu address: $7FFD
	{
		videopage = byte << 4;
		return;
	}

	if ((addr & 0x74) == 0x34) // enable NMI button, if A7=0 deactivate/hide MF128
	{
		nmi_pending = no;
		if ((addr & 0x80) == 0) mf_enabled = no;
	}
}


/*	handle rom patch
	test if address = 0x0066 and pages in MF128 memory
		except if nmi_button still enabled, then s.o. else has pressed nmi
			except if activate_on_any_nmi is set
	rearside ROMCS is ignored by the MF128
	note: executing address 0x0066 does not disable the IF128 NMI button
		this is done by the press of the IF128 NMI button itself
		the MF128 can still generate an NMI if this NMI was generated by some other device.
		therefore disabling the button is done in pushNmiButton(), not here.
	returns new opcode
*/
uint8 Multiface128::handleRomPatch(uint16 pc, uint8 o)
{
	if ((pc | 1) != 0x0067 || !nmi_pending) // not NMI address or NMI not triggered by me
		return prev()->handleRomPatch(pc, o);

	assert(mf_enabled); // nmi_pending should imply mf_enabled

	if (!paged_in) page_in();
	return rom[pc];
}


/*	press the red button
 */
void Multiface128::triggerNmi()
{
	if (nmi_pending) return;
	nmi_pending = yes;
	mf_enabled	= yes;
	machine->cpu->triggerNmi();
}
