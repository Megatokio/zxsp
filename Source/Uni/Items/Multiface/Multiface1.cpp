// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface1.h"
#include "Items/Item.h"
#include "Items/Joy/KempstonJoy.h"
#include "Machine.h"
#include "Memory.h"
#include "Settings.h"


//    WoS:
//    #define P_MF1_IN                        0x9f    /* Port address */
//    #define P_MF1_OUT                       0x1f    /* Port address */
//
//  8k ROM
//  2k RAM

/*  WoS:

Developed by Romantic Robot.
Several different models were produced for use with different systems, as indicated by their names.

The Multiface requires no system memory; it has 8K EPROM and 8K RAM available on-board (Multiface 1 has 2K RAM only),
and needs no software to operate. One of the most appealing features is the ability to transfer programs to
Microdrive, Disk, Wafadrive or Tape easily and quickly; saved programs can be compressed, and can be re-loaded
without a Multiface being attached. In addition, it is possible to read through the contents of memory at any time,
making the Multiface popular with games players and developers alike.

Joystick interface – Kempston compatible (IN 31)

1) Push the red button to activate MULTIFACE ONE
2) Select function by pressing the relevant key (inversely printed) of the 6 commands displayed:
exit – to exit to BASIC to either:
	a) leave the program and MULTIFACE ONE entirely
	b) study/alter/customize the program
	All efforts are made to preserve program intact. The main pre-condition is the existence of standard
	system variables or else the Spectrum will crash. Successful exit gives full access to a program.
	To restart it, if needed, you need to know starting line or address. By re- activating the MULTIFACE,
	a program can then be saved, etc.
return – to continue the program
save – to proceed to SAVE routines:
	a) Input the name of program – up to 9 characters or just press ENTER to input RUN automatically;
	   BETA accepts 7 characters or defaults to BOOT.
	b) Choose saving to: tape, cartridge, wafer, disc MULTIFACE automatically detects if DISCOVERY or BETA
	   is attached & follows the right procedure. You can save program or screen only by pressing p or s.
	   Programs are automatically compressed to take less room and load faster.
	   Screens are left intact, as they would need an expanding program.
tool – to access MULTI TOOLKIT routines
	quit – to return to the opening menu
	ENTER+SPACE – to PEEK/POKE; SPACE clears address
	hex – to toggle between hex and decimal display
	reg – to display registers (held at 16358-16383)
	window – to open window to show 128 bytes. Select poking address (flashing) with cursor keys.
	text – to see the window in ASCII characters
copy – to COPY screen to a printer: for interfaces with COPY command (Kempston E, Lprint III, etc.)
jump – not to return but jump to another address. The address to jump is at 8192 & 8193 (low, hi).
	You can jump to Spectrum ROM/RAM & to M1 8K RAM. As MULTIFACE RAM overshadows ZX ROM (8192-16383)
	address 8194 determines paging status: if 0, 8K RAM remains paged, poking 1 unpages the RAM:
	any other value disables the jump command entirely.


Deduced from from Circuit:

	Bits "-001.--1-" aktivieren das MF1:

	in(31):	Bit 7=1 aktiviert das MF-Ram+Rom (gleichzeitig wirf der JS gelesen)
			Bit 7=0 liest den Joystick (gleichzeitig wird das MF-Ram+Rom deaktiviert)


	Reset:	Blendet das MF-Ram+Rom aus
			schaltet den NMI-Taster scharf

	out(31):schaltet den NMI-Taster scharf

	Taster:	erzeugt NMI
			deaktiviert den Taster

	execute 0x66: blendet das MF-Ram+Rom ein

	rear-side ROMCS ist nicht angeschlossen!

	Wenn das MF-Ram+Rom ausgeblendet ist, ist das Ram auch nicht mehr beschreibbar
	Per Jumper konnte der Joystick deaktiviert werden, z.B. bei Problemen mit einem Diskinterface.

	Hmm...: Der Jumper wirkte nur auf die Bits 6 und 7. -> Bug oder Absicht?
		Der Joystick wäre also mit offenem Jumper immer noch lesbar, nur nicht mehr ganz Kempston-konform.
		Bits 6 und 7 könnten von einem anderen Device gesetzt werden.
*/


static cstr o_addr = "----.----.-001.--1-"; //	read Kempston Joystick, page ram+rom if bit7=1
static cstr i_addr = "----.----.-001.--1-"; //	nmi-taster wieder scharf schalten


Multiface1::Multiface1(Machine* m) :
	Multiface(m, isa_Multiface1, "Roms/mf1.rom", o_addr, i_addr), joystick(nullptr), overlay(nullptr),
	joystick_enabled(settings.get_bool(key_multiface1_enable_joystick, yes))
{
	insertJoystick(usb_joystick0);
}

Multiface1::~Multiface1() { machine->removeOverlay(overlay); }


void Multiface1::powerOn(/*t=0*/ int32 cc)
{
	Multiface::powerOn(cc);
	machine->rom[0x0066] |= cpu_patch;
	machine->rom[0x0067] |= cpu_patch;
}


// void Multiface1::reset( Time t, int32 cc )
//{
//	Multiface::reset(t,cc);
// }


/*	Input at registered address: 0x1F (only 4 bits decoded)
	read Kempston Joystick
	page ram+rom if bit7=1
*/
void Multiface1::input(Time, int32, uint16 addr, uint8& byte, uint8& mask)
{
	assert((addr & 0x72) == 0x12);

	// read joystick: %000FUDLR active high
	// Jumper wirkt laut Schaltplan nur auf Bit 6 und 7!
	// Laut MF1 Instructions aber auf den ganzen Port. => Schaltplan Fehler?
	uint8 js_byte = machine == front_machine ? joystick->getState(yes) : 0x00;
	if (joystick_enabled)
	{
		mask = 0xff;
		byte = js_byte;
	}
	// else				 { mask |= 0x3f; byte &= js_byte | 0xC0; }		wir glauben mal dem Manual

	// page memory in/out:
	if (addr & 0x80)
	{
		if (!paged_in) page_in();
	}
	else
	{
		if (paged_in) page_out();
	}
}


/*	Output at registered address:
	nmi-taster wieder scharf schalten
*/
void Multiface1::output(Time, int32, uint16 addr, uint8)
{
	assert((addr & 0x72) == 0x12);
	nmi_pending = no;
}


/*	handle rom patch
	test if it is 0x0066 or 0x0067 and page in MF1 memory
	rearside ROMCS is ignored by the MF1
	returns new opcode
*/
uint8 Multiface1::handleRomPatch(uint16 pc, uint8 o)
{
	if ((pc | 1) != 0x0067 || !nmi_pending) return prev()->handleRomPatch(pc, o); // not me

	if (!paged_in) page_in();
	return rom[pc];
}


/*	press the red button
 */
void Multiface1::triggerNmi()
{
	if (nmi_pending) return;
	nmi_pending = yes;
	machine->cpu->triggerNmi();
}


void Multiface1::insertJoystick(int id) volatile
{
	if (joystick == joysticks[id]) return;

	if (overlay)
	{
		machine->removeOverlay(overlay);
		overlay = nullptr;
	}
	joystick = joysticks[id];
	if (id != no_joystick) overlay = machine->addOverlay(joystick, "K", Overlay::TopRight);
}
