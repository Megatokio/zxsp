// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SmartSDCard.h"
#include "Machine.h"
#include "Memory.h"
#include "Settings.h"
#include "Z80/Z80.h"
#include "kio/kio.h"

/*	Questions:

	The onboard FlashRAM chip is type 39SF020. The write protocol for this chip requires address $2AAA (and $5555)
	to be written before each programmed byte (which is why video “glitches” can be seen whilst data is written
	to the chip). [Manual]

	Although the 39SF020 EEPROM has addressing just like a RAM, it cannot just accept writes like one.
	It requires that addresses $2AAA and $5555 are written with various byte sequences first (before the actual
	data byte/address write) to overcome it's accidental write lock-out mechanisms.

	The way the SMART card handles Flash Rom writes is as follows:

	A0:A13 go direct to the EEPROM.

	A14 goes direct to the EEPROM during writes but for reads it's sourced from the ROM bank register.
	This allows straight forward CPU writes to addresses $5555 and $2AAA to be seen by the EEPROM.

	The EEPROM's addressing layout is linear, but because A14 is under direct CPU control during writes and
	the bank register sets its other upper address lines I handle things as follows:

	Writes that need to go to even 16KB banks have to be written to $0000-$3fff in Z80 address space.
	Writes that go to odd banks have to be written to $4000-$7fff.
	There's no way to stop these bytes also going to the Spectrum itself. $0000-$3fff obviously has no
	effect on the Spectrum :) and $4000-$7fff messes up the display, but can be easily undone.
	(I guess they could also go to $8000-$bfff / $c000-$ffff but I generally use upper RAM for the code.)	[Phil]



*/

/*
	Blue LED:
		The blue LED is on whenever the SMART card's memory is paged in
		the software enable/disable bit or the rom page-out mechanism also affect it, but DIP 1 is king. [Phil]

	Red LED:
		The red LED is controlled by DIP 2 (Enable writing to FlashRAM) only. (The EEPROM must be writable even
		if the SMART memory is disabled (by DIP 1 etc) so that a new (or corrupt) card can be flashed). [Phil]

	DIP SW 1: ON: interface memory can be paged in
		the blue LED is lit when the PSI card is paged in.
		does not override DIP 2 (writing to flash). [Phil]
		overrides DIP 3 (Bank B Override). [Phil]
		disables writing to ram. [Phil]
		does not disable the SD card interface. [Phil]
		does not disable the RS232 interface. [Phil]

	DIP SW 2: ON: Enable writing to FlashRAM
		the red LED is lit when writes are enabled / when this switch is ON.
		Overrides DIP 1. Only DIP 2 has the say when it comes to enabling writes to Flash.
		Flash must be writable without the SMART ROM being active or there'd be no way to set up a new card
		(apart from burning in an external writer before soldering) [Phil]

	DIP SW 3: ON: Bank B Override:
		bypass the ROM Manager and start directly from Slot B (e.g. diagnostic rom).
		When this switch is ON and DIP 1 is ON, then the card starts directly from - and can only access - ROM slot B.
		This switch does no more than override bits 0:3 in port $FAFB (FlashROM selection). [Phil]

	DIP SW 4: used on PCB versions 1.05 and above to enable/disable the Kempston joystick port. Previously unused.
		Setting this switch to ON enables the joystick interface. [Phil]

	ROM: 16 x 16 kB
	All ROM files (except the FIRMWARE) are 16KB.
	The FIRMWARE.Vxx file is shorter (12KB) because the final 4KB of SLOT A in the EEPROM is used
	for the ROM index etc and is not to be overwritten.

	Writing to Flash EEPROM:
		The onboard FlashRAM chip is type 39SF020. The write protocol for this chip requires address $2AAA
		and $5555 to be written before each programmed byte (which is why video “glitches” can be seen whilst
		data is written to the chip). Enabling SRAM at $2000-$3FFF prevents the FlashRAM chip being selected
		for address $2AAA so it cannot be written to in this mode.

		Although the 39SF020 EEPROM has addressing just like a RAM, it cannot just accept writes like one.
		It requires that addresses $2AAA and $5555 are written with various byte sequences first before the actual
		data byte/address write to overcome it's accidental write lock-out mechanisms.

	The way the SMART card handles Flash Rom writes is as follows:

		A0:A13 go direct to the EEPROM.

		A14 goes direct to the EEPROM during writes but for reads it's sourced from the ROM bank register.
		This allows straight forward CPU writes to addresses $5555 and $2AAA to be seen by the EEPROM.

		The EEPROM's addressing layout is linear, but because A14 is under direct CPU control during writes and
		the bank register sets its other upper address lines I handle things as follows:

		Writes that need to go to even 16KB banks have to be written to $0000-$3fff in Z80 address space.
		Writes that go to odd banks have to be written to $4000-$7fff.
		There's no way to stop these bytes also going to the Spectrum itself. $0000-$3fff obviously has no
		effect on the Spectrum :) and $4000-$7fff messes up the display, but can be easily undone.
		(I guess they could also go to $8000-$ffff but I generally use upper RAM for the code.)		[Phil]

	Rom switch-out system:
		When $FAFB bit 6 is set, the PSI card's memory is automatically paged out when the Z80 reads from
		address $xx72. Only bits 6,5,4 and 1, /MREQ and /RD are decoded. [Phil]
		M1 is not connected to the SMART Card (I was wary of incompatibility - Spectrum are
		notorious for dodgy M1 signals.) [Phil]
		does it immediately switch out and already read from address 0x0072 in ZX Spectrum rom?
		There's a bit of syncing that goes on. First: A6, A5, A4, A1, MREQ, RD, Primer(Bit6_$FAFB) are AND'ed
		(MREQ and RD are inverted) to set a flip-flop which is clocked by the SMART card's 16MHz Oscillator.
		The output of this flip-flop goes to input of another flip-flop which has it's enable connected to MREQ
		(also clocked by the 16MHz oscillator) - IE: it can only change when MREQ is inactive. [Phil]
		Kio: this probably means that roms are switched before the Z80 reads it's byte.

	RAM: 16 x 8 kB
	reading and writing to ram:
	ram is only writable when it is readable. [Phil]
		DIP 1 (internal Spectrum ROM disable) must be set.
		port $FAFB bit 7 (disable interface memory) must be cleared '0'.
		port $FAF3 bit 7 (enable ram) must be set '1'.


	port $FAFB: FlashROM selection / switching
		read / write port
		cleared on power up or on reset
		all address bits decoded
	Bit 0:3 - Select the 16KB section of FlashRAM that appears to the CPU @ $0000-$3FFF
			  When DIP Switch 3 is set to ON, this value is ignored and slot B always selected.
		4,5 - Not used, reads back as written.
		6 - Prime the ROM switch-out system for restarting snapshot files
			When set, the PSI card's memory is automatically paged out when the Z80 reads from
			address $xx72. (see above note on Rom switch-out system.)
			When reading, this bit always returns 0.
		7 - Disable the card’s memory / use Spectrum's internal ROM
			setting bit 7 prevents both the SRAM and EEPROM from being selected. [Phil]

	port $FAF3: SRAM selection
		read/writeable port
		cleared upon reset
		all address bits decoded
	Bit 0:3 – Selects which 8KB bank of SRAM appears at $2000-$3FFF (when enabled)
		4 – Serial TX when written to, Serial RX when read.
			reads '1' if no serial interface connected. CPLD has a "KEEPER" option which stops it floating. [Phil]
		5 – AUX SPI_CS signal (see 4x2 header) 1 to select, 0 to deselect (output is inverted by PCB)
		6 – SD Card CS control (write 1 to select SD card, 0 to deselect) – also activates green access LED
		7 – SRAM enable. When set, SRAM replaces FlashRAM in memory locations $2000-$3FFF.
			Note if the PSI card's memory is disabled, this has no effect.
			Note: The onboard FlashRAM chip is type 39SF020. The write protocol for this chip requires
			address $2AAA (and $5555) to be written before each programmed byte
			(which is why video “glitches” can be seen whilst data is written to the chip).
			Enabling SRAM at $2000-$3FFF prevents the FlashRAM chip being selected for address $2AAA
			so it cannot be written to in this mode.

	port $FAF7 Data to/from the SD card
		read/write port
		all address bits decoded
	Bit 0:7 Data for SD card
			There is no serializer busy flag: make sure at least 12 Z80 cycles elapse between accesses.
			reads $FF if no sd card is fitted. [Phil]
			reads $FF if reading more bytes than sent by the SD card. [Pretty sure.. Phil]

	port $1F: Kempston joystick
		address bits 7:0 are decoded.
		Bits that are set indicate that a direction is selected
		Bit 0 – Right
			1 – Left
			2 – Down
			3 – Up
			4 - Button 1
			5 - zero
			6 - zero
			7 - Button 2


	Left button: reset button
	Right button: NMI

	Connectors / Pin headers:
	The Joystick DSUB-9 pins are connected as follows:
		1 – Up (10K pull-up)
		2 – Down (10K pull-up)
		3 – Left (10K pull-up)
		4 – Right (10K pull-up)
		5 – 3.3v (via 47 ohm resistor)
		6 – Button 1 (10K pull-up)
		7 – 3.3 v (via 47 ohm resistor) 8 – GND
		9 – Button 2 (10K pull-up)

	The 4x2 pin header has the following layout / pin-outs:
		1357
		2468
		1 – Serial TX RS232 level output (where optional ST232 is fitted)
		2 – 5 volts
		3 – SPI_CS
		4 – Serial RX RS232 level input (where optional ST232 chip is fitted)
		5 – SPI D_out
		6 – SPI D_im 7 - GND
		8 – SPI_Clock
			(The SPI bus lines D_out, D_in and SPI_Clock are shared with the SD card,
			the SPI_CS line is dedicated to this port).

	The 6x1 pin header is for JTAG configuration of the CPLD

	The 3x1 pin header is used to select ROM_CS line on the edge connector and must be set for the host machine.
		123
		• When the jumper is across 1-2, the Spectrum model is Spectrum +2B or +3
		• When the jumper is across 2-3, the Spectrum model is Original Spectrum 48k, 128, or +2

*/

#define _port_flashrom 0xfafb // 1111.1010.1111.1011		all bits decoded - confirmed
#define _port_ram	   0xfaf3 // 1111.1010.1111.0011		all bits decoded - confirmed
#define _port_sdcard   0xfaf7 // 1111.1010.1111.0111		all bits decoded - confirmed
#define _port_joystick 0x001f // ----.----.0001.1111		8 bits decoded - confirmed
#define _mem_switchout 0x0072 // ----.----.-111.--1-		bits 6,5,4 and 1 are decoded

#define o_addr "1111.1010.1111.--11"
#define i_addr ""

// #define key_smart_card_joystick_enabled		"settings/smart_card_joystick_enabled"		// bool
// #define key_smart_card_memory_enabled			"settings/smart_card_memory_enabled"		// bool
// #define key_smart_card_force_bank_B			"settings/smart_card_force_bank_B"			// bool
// #define key_smart_card_write_flash_enabled	"settings/smart_card_flash_write_enabled"	// bool

/* ----------------------------------------------------------------------------------

	use of machine.cpu_options: cpu_patch | cpu_memmapped_r | cpu_write_2x | cpu_memmapped_w
	flash_dummy_page[CPU_PAGESIZE]

  •	Das Flash ist beschreibbar wenn dip_flash_write_enabled=ON und rom_config.bit7=0

	Init:
		cpu_memmapped_w setzen
		in flash_dummy_page[] cpu_memmapped_w setzen
		flash_dummy_page[] überall als writepage2 eintragen (das überlebt auch cpu.init())

	Beschreibbar:
		cpu_write_2x setzen
		für gesamten Adressraum cpu.Wom2 auf flash_dummy_page[] setzen
		=> writeMemory() wird aufgerufen

	Nicht beschreibbar:
		cpu_write_2x löschen
		für gesamten Adressraum cpu.Wom2 löschen
		=> writeMemory() wird nicht aufgerufen
		note: machine.cpu_options bit cpu_write_2x löschen geht nicht,
			  weil das in einem output() nicht sofort wirkt.

	Note:
		Das SmartSDCard Interface ist damit inkompatibel mit allen anderen Interfaces,
		die cpu_write_2x benutzen, z.B. das SPECTRA Video Interface.

  •	Das Rom Switch-out System wird von Memory Reads auf Adresse 0x72 aktiviert.
	Dabei werden nur die 4 gesetzten Bits und MREQ und RD dekodiert.

	Init:
		cpu_patch | cpu_memmapped_r setzen

	Aktiv:
		In den 4096 sichtbaren Adressen wird cpu_patch | cpu_memmapped_r gesetzt.
		=> handleRomPatch() oder readMemory() wird aufgerufen.

	Inaktiv:
		In den 4096 sichtbaren Adressen wird cpu_patch | cpu_memmapped_r gelöscht.
		=> handleRomPatch() oder readMemory() wird nicht aufgerufen.

	Änderungen an der Speicherkonfiguration:
		Im ausgeblendeten Speicher werden die Bits gelöscht,
		im eingeblendeten Speicher werden sie gesetzt.

		Problem: Änderung der Speicherkonf. *vor* dem Interface, z.B. im ZX Spectrum selbst.
		Das kann man ggf. ignorieren, weil scharf machen und auslösen fast immer direkt hintereinander erfolgen,
		weil 1/16 des Speichers "verseucht" ist.
		Aber: es gibt immer wieder 16*7 Byte große Bereiche, die nicht gesprayt sind.
		Außerdem ist "scharfmachen, schnell noch ein OUT für die Speicherkonfig und dann raus" durchaus denkbar.

		Lösung: *allen* Speicher sprayen:
		- alle Roms (eigene 256K + 16/32/64K internes Rom + ggf. dazwischen hängende Interfaces)
		- alle Rams (eigene 128K + bis zu 128K internes Ram + Ram in dazwischen hängenden Interfaces)

		Lösung: spezielles Bit in cpu_options
		- aller Speicher müsste (einmal) gesprayt werden
		- setzen/löschen der cpu_option z.zt. nicht in der laufenden CPU möglich.

		Lösung: neue Funktion im Item-Interface:
		- Rückruf bei Speicherkonfig-Änderungen

		Das ist alles brackig.
		Wenn das Original-Rom keinen unerwarteten Unfug macht, dann wird das Problem bis auf weiteres ignoriert.

---------------------------------------------------------------------------------- */


static CoreByte flash_dummy_page[CPU_PAGESIZE];


// ================================================================================
//								static helper
// ================================================================================


inline CoreByte* clear_bits_in_page(CoreByte* p, uint32 bitmask)
{
	if (*p & bitmask)
	{
		bitmask = ~bitmask;
		for (CoreByte* e = p + CPU_PAGESIZE; p < e; p++) { *p &= bitmask; }
	}
	return p;
}

inline CoreByte* set_bits_in_page(CoreByte* p, uint32 bitmask)
{
	if (~*p & bitmask)
	{
		for (CoreByte* e = p + CPU_PAGESIZE; p < e; p++) { *p |= bitmask; }
	}
	return p;
}


// ================================================================================
//									ctor / dtor
// ================================================================================


SmartSDCard::SmartSDCard(Machine* m, uint dip_switches) :
	MassStorage(m, isa_SmartSDCard, external, o_addr, i_addr),
	ram(m, "SMART card Ram", 128 kB),
	rom(m, "SMART card Flash Ram", 256 kB),
	joystick(nullptr),
	overlay(nullptr),
	sd_card(nullptr),
	sio(nullptr),
	config(),
	//	dip_joystick_enabled(gui::settings.get_bool(key_smart_card_joystick_enabled, yes)),
	//	dip_memory_enabled(gui::settings.get_bool(key_smart_card_memory_enabled, yes)),
	//	dip_force_bank_B(gui::settings.get_bool(key_smart_card_force_bank_B, no)),
	//	dip_flash_write_enabled(gui::settings.get_bool(key_smart_card_write_flash_enabled, no)), flash_state(),
	dip_joystick_enabled(dip_switches & Dip::JoystickEnabled),
	dip_memory_enabled(dip_switches & Dip::MemoryEnabled),
	dip_force_bank_B(dip_switches & Dip::ForceBankB),
	dip_flash_write_enabled(dip_switches & Dip::FlashWriteEnabled),
	flash_state(),
	flash_dirty(no),
	flash_software_id_mode()
{
	// init rom:
	// read patches will only be needed for rom switch-out system and are only applied if it is armed, not in general.
	// memory writes will never hit the rom pages, they hit the flash_dummy_page instead. so don't set these patch bit.
	CoreByte* p = rom.getData();
	CoreByte* e = p + rom.count();
	while (p < e) *p++ = 0x000000ff;
	rom[0] = rom[1] = cpu_memmapped_r | cpu_patch; // for software ID mode

	FD	   fd(catstr(appl_rsrc_path, "smart_sdcard.rom"));
	uint32 flen = fd.file_size();
	uint8  zbu[flen];
	fd.read_bytes(zbu, flen);
	m->cpu->b2c(zbu, rom.getData(), flen);

	// TODO: evtl. load custom rom
	// TODO: evtl. save/restore $3000 - $3fff

	if (dip_joystick_enabled) insertJoystick(usb_joystick0);
}


SmartSDCard::~SmartSDCard()
{
	delete sd_card;
	delete sio;
	machine->removeOverlay(overlay);

	// TODO: save modifiactions to rom
}


void SmartSDCard::powerOn(/*t=0*/ int32 cc)
{
	MassStorage::powerOn(cc);
	config				   = memory_disabled; // will be enabled by set_rom_config()
	flash_state			   = flash_idle;
	flash_software_id_mode = no;
	machine->cpu_options |= cpu_patch | cpu_memmapped_r | cpu_memmapped_w;

	// init flash_dummy_page[]:
	// used for writing to flash in flash command mode: cpu.page2[]  -> cpu_memmapped_w
	// used for reading from flash while flash write in progress	 -> cpu_memmapped_r | cpu_patch
	set_bits_in_page(flash_dummy_page, cpu_memmapped_w | cpu_memmapped_r | cpu_patch);

	// potentielle Speicherstellen für Adresse $5555 patchen,
	// damit beim Beschreiben der Flash Command Handler aufgerufen wird:
	// Als minimale Page-Size wird 8kB (0x2000) angenommen.
	for (uint i = 0; i < machine->memory.count(); i++)
	{
		CoreByte* p = machine->memory[i]->getData();
		CoreByte* e = p + machine->memory[i]->count();
		while (p < e)
		{
			p[0x5555 & 0x1FFF] |= cpu_memmapped_w;
			p += 0x2000;
		}
	}

	// clear the rom page-out trap bits:
	clear_rom_pageout_bits();

	// init attached devices:
	if (sio) sio->init();
	if (sd_card) sd_card->init();

	// spray write2x bit if flash write enabled, else remove it:
	set_dip_flash_write_enabled(dip_flash_write_enabled);

	// page-in card memory if DIP 1 enabled:
	set_rom_config(0);
}


void SmartSDCard::reset(Time t, int32 cc)
{
	MassStorage::reset(t, cc);
	set_ram_config(t, 0);
	set_rom_config(0);
}


// ================================================================================
//									helper
// ================================================================================


/*	set or clear dip_flash_write_enabled:
	write2x bit in allem Speicher setzen bzw. löschen
	write2x bit in cpu_options setzen / löschen
	Note: dummy_write_pages in anderen Erweiterungen werden nicht gesprayt, if any.
*/
void SmartSDCard::set_dip_flash_write_enabled(bool f)
{
	dip_flash_write_enabled = f;

	if (f) // enable
	{
		//		machine->cpu_options |= cpu_write_2x;

		// cpu.nowritepage[] impfen:
		//		set_bits_in_page(machine->cpu->nowritepage,cpu_write_2x);

		// note: die eigene flash_dummy_page muss nicht gesprayt werden,
		// weil sie nie als Write Page gemappt wird, sondern nur als Read Page oder als Wom2 Page.

		// Allen Speicher impfen:
		//		for(uint i=0; i<machine->memory.count(); i++)
		//		{
		//			CoreByte* p = machine->memory[i]->getData();
		//			CoreByte* e = p + machine->memory[i]->count();
		//			while(p<e) { *p++ |= cpu_write_2x; }
		//		}

		// memory enabled  => flash write enabled
		if (~config & memory_disabled) enable_flash_write();
	}
	else // disable
	{
		//		machine->cpu_options &= ~cpu_write_2x;

		// bits in cpu.nowritepage[] löschen:
		//		clear_bits_in_page(machine->cpu->nowritepage,cpu_write_2x);

		// Bits im gesamten Speicher wieder löschen:
		//		for(uint i=0; i<machine->memory.count(); i++)
		//		{
		//			CoreByte* p = machine->memory[i]->getData();
		//			CoreByte* e = p + machine->memory[i]->count();
		//			while(p<e) { *p++ &= ~cpu_write_2x; }
		//		}

		// flash write disabled:
		if (~config & memory_disabled) disable_flash_write();
	}
}

/*	map flash rom into cpu address space for writing
	note: if flash is writable (DIP 2) and card memory is enabled (rom_config bit 7)
		  then the flash rom is writable in the whole cpu address space.
	if no flash command is in progress, then the flash must be written to address $5555 to start a command.
		(exception: if software ID mode pending, then this mode can be terminated by writing to ANY address.)
		then we rely on the cpu_option cpu_memmapped_w sprayed to any potential address in machine memory.
		so the machine is only slowed down if address %5555 (or a potential mirror) is written to.
	if a command is in progress, then we must process any flash write, either to proceed command or to bail out.
		this is done by mapping out flash_dummy_page for secondary write page into entire cpu address space.
		The flash_dummy_page[] has cpu_option cpu_memmapped_w set in all bytes, so writeMemory() gets always called.
	zxsp will be slowed down to a crawl if software ID mode is entered and never reset.
*/
inline void SmartSDCard::enable_flash_write()
{
	assert(dip_flash_write_enabled);
	assert(~config & memory_disabled);

	// Wom2 muss nur reingemappt werden, wenn wir im command mode sind:
	if (flash_software_id_mode || flash_state > flash_idle)
	{
		// darf nicht ausgeschaltet worden sein, weil es sich hier nur verzögert wieder einschalten ließe!:
		// machine->cpu_options |= cpu_write_2x;
		machine->cpu->mapDummyWom2Page(0, 0, flash_dummy_page);
	}
}

inline void SmartSDCard::disable_flash_write()
{
	// Wom2 ist nur reingemappt, wenn wir im command mode sind:
	if (flash_software_id_mode || flash_state > flash_idle)
	{
		// darf nicht ausgeschaltet werden, weil es sich nur verzögert wieder einschalten lässt!:
		// machine->cpu_options &= ~cpu_write_2x;
		machine->cpu->unmapWom2(0, 0);
	}
}

/*	start flash write mode
	setup for software polling of flash write end
*/
void SmartSDCard::start_flash_write()
{
	if (flash_state == flash_writing) return; // false call

	flash_state = flash_writing; // set flag
	if (!flash_software_id_mode) machine->cpu->unmapWom2(0, 0);

	flash_dirty = yes;

	// map the patched flash_dummy_page[] for reading for software polling for flash write end:
	if (memory_enabled()) map_card_memory(0, 0x4000);
}

/*	finish flash write mode
	restore normal flash rom reading
*/
void SmartSDCard::finish_flash_write()
{
	if (flash_state != flash_writing) return; // false call
	flash_state = flash_idle;				  // clear flag

	// unmap the patched flash_dummy_page[] for reading from flash:
	if (memory_enabled()) map_card_memory(0, 0x4000);
}

void SmartSDCard::map_card_memory(uint a, uint e)
{
	assert(memory_enabled());

	if (config & pageout_armed) clear_rom_pageout_bits(a, e);

	uint m = config & ram_enabled ? 0x2000 : 0x4000;

	if (a < m)
	{
		if (flash_state == flash_writing) { machine->cpu->mapDummyRomPage(a, m - a, flash_dummy_page, nullptr, 0); }
		else
		{
			uint32 romaddr = dip_force_bank_B ? 0x4000 : (config & rompage_bits) << 6;
			machine->cpu->mapRom(a, m - a, &rom[romaddr + a], nullptr, 0);
		}
	}

	if (m < e)
	{
		uint32 ramaddr = (config & rampage_bits) << 13;
		machine->cpu->mapRam(m, e - m, &ram[ramaddr], nullptr, 0);
	}

	if (config & pageout_armed) set_rom_pageout_bits(a, e);
}


/*	add cpu_patch bits to rom-switch-out addresses
	a = start address (must start on cpu page boundary)
		normally 0x0000, may be 0x2000 if only card ram is affected
	e = end address
		normally 0xffff, may be 0x3fff if only card memory is affected
	address = $0072 = "----.----.-111.--1-"		only bits 6,5,4 and 1 are decoded
	rom page-out bits are applied to _visible_ pages.
		if memory config changes, these bits must be updated!
		note: cannot be handled for internal memory of ZX Spectrum...
		so we will just handle our own memory mapping and ignore the rest (for now).
*/
void SmartSDCard::set_rom_pageout_bits(uint a, uint e)
{
	Z80* cpu = machine->cpu;
	for (; a < e; a += CPU_PAGESIZE)
	{
		CoreByte* pg = cpu->rdPtr(a);
		if (pg[0x72] & cpu_memmapped_r) continue; // bits in this page already set (assuming whole page)

		for (uint i = 0; i < CPU_PAGESIZE; i += 0x80) // loop over groups of 0x80 bytes
		{
			CoreByte* m = pg + i;
			m[0x72] |= cpu_patch | cpu_memmapped_r;
			m[0x73] |= cpu_patch | cpu_memmapped_r;
			m[0x76] |= cpu_patch | cpu_memmapped_r;
			m[0x77] |= cpu_patch | cpu_memmapped_r;
			m[0x7a] |= cpu_patch | cpu_memmapped_r;
			m[0x7b] |= cpu_patch | cpu_memmapped_r;
			m[0x7e] |= cpu_patch | cpu_memmapped_r;
			m[0x7f] |= cpu_patch | cpu_memmapped_r;
		}
	}
}

/*	clear cpu_patch bits at all rom-switch-out positions
	note: due to the risk that this might also reset a bit for some other device
		  this should not be called while the ZX Spectrum rom is paged in
*/
void SmartSDCard::clear_rom_pageout_bits(uint a, uint e)
{
	Z80* cpu = machine->cpu;

	// preserve bits in flash_dummy_page[] which is mapped for reading while flash write is in progress:
	while (cpu->rdPtr(a) == flash_dummy_page) a += 0x2000;

	for (; a < e; a += CPU_PAGESIZE)
	{
		CoreByte* pg = cpu->rdPtr(a);
		if (~pg[0x72] & cpu_memmapped_r) continue; // bits in this page already cleared (assuming whole page)

		for (uint i = 0; i < CPU_PAGESIZE; i += 0x80) // loop over groups of 0x80 bytes
		{
			CoreByte* m = pg + i;
			m[0x72] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x73] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x76] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x77] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x7a] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x7b] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x7e] &= ~cpu_patch & ~cpu_memmapped_r;
			m[0x7f] &= ~cpu_patch & ~cpu_memmapped_r;
		}
	}
}


// ================================================================================
//							input / output / memory patches
// ================================================================================


void SmartSDCard::input(Time t, int32, uint16 addr, uint8& byte, uint8& mask)
{
	// memory config or SD card data:
	if ((addr & 0xfff3) == 0xfaf3)
	{
		switch ((addr >> 2) & 3)
		{
		case 0b00: // $FAF3: RAM config
			mask = 0xff;
			if (sio) byte &= (config & ~sio_rx) | sio->input(t) * sio_rx;
			else byte &= config | sio_rx; // bit 4 (SIO RX) reads '1' if no SIO fitted
			break;
		case 0b01: // $FAF7: SD card data
			mask = 0xff;
			// reads $FF if not selected, no card, or reading beyond input data:
			if (sd_card && (config & sdcard_cs)) byte &= sd_card->input();
			break;
		case 0b10: // $FAFB: ROM config
			mask = 0xff;
			byte &= (config & ~pageout_armed) >> 8; // bit 6 (PAGEOUT PRESET) always reads 0
			break;
		}
		return;
	}

	// Kempston joystick interface:
	// bits 7:0 are decoded
	// TODO: bit 7 = button 2
	if ((addr & 0x00ff) == 0x1f && dip_joystick_enabled)
	{
		// Input: %000FUDLR  active high
		mask = 0xff;
		byte &= machine == front_machine ? joystick->getState(yes) : 0x00;
		return;
	}
}


void SmartSDCard::output(Time t, int32, uint16 addr, uint8 byte)
{
	if ((addr & 0xfff3) == 0xfaf3)
	{
		switch ((addr >> 2) & 3)
		{
		case 0b00:
			if (byte != uint8(config)) set_ram_config(t, byte);
			break; // $FAF3: RAM config
		case 0b01:
			if (sd_card && (config & sdcard_cs)) sd_card->output(byte);
			break; // $FAF7: SD card data
		case 0b10:
			if (byte != (config >> 8)) set_rom_config(byte << 8);
			break; // $FAFB: ROM config
		}
	}
}


/*	port $FAF3: set ram configuration
	current assumption:
		RAM is writable if it is readable
		RAM is paged in
			if FAF3.7 = 1
			if FAFB.7 = 0
			if DIP 1 is set
		other settings do not affect the page-in state of the RAM
*/
void SmartSDCard::set_ram_config(Time t, uint new_config)
{
	new_config |= (config & 0xFF00);
	uint old_config = config;
	uint toggled	= new_config ^ old_config; // x = toggled bits
	config			= new_config;

	if (toggled & sio_tx) // RS232 TX bit				(if fitted)
	{
		if (sio) sio->output(t, new_config & sio_tx);
	}

	// if(toggled&aux_cs)	// AUX SPI_CS; 1=select		(if fitted)
	//{}					// not supported

	// if(toggled&cscard_cs)	// SD Card CS; 1 = select
	//{}					// handled in input() / output()

	// if ram enabling or selected ram page changed
	if (memory_enabled() && (toggled & (ram_enabled | rampage_bits))) map_card_memory(0x2000, 0x4000);
}


/*	port $FAFB: set rom configuration
	note: config bits must be in bits 15:8
*/
void SmartSDCard::set_rom_config(uint new_config)
{
	new_config |= (config & 0x00FF); // add current ram_config
	uint old_config = config;
	uint toggled	= new_config ^ old_config; // toggled bits
	config			= new_config;

	if (!dip_memory_enabled) return; // memory disabled by dip switch

	// bits 0-3: page
	// bit  4,5: n.c.
	// bit  6:   1 = Prime the ROM switch-out system
	// bit  7:   1 = Disable the card memory

	Z80* cpu = machine->cpu;

	if (new_config & memory_disabled)
	{
		// disable card memory:

		// was enabled? => page out
		if (toggled & memory_disabled)
		{
			if (old_config & pageout_armed) clear_rom_pageout_bits();
			if (old_config & ram_enabled) cpu->unmapWom(0x2000, 8 kB); // unmap ram write page
			if (dip_flash_write_enabled) disable_flash_write();
			prev()->romCS(off); // enable ZX Spectrum rom
		}
	}
	else
	{
		// enable card memory:

		// was disabled? => page in:
		if (toggled & memory_disabled)
		{
			prev()->romCS(on);								   // switch off ZX Spectrum rom
			if (dip_flash_write_enabled) enable_flash_write(); // flash rom becomes writable

			if (new_config & pageout_armed) set_rom_pageout_bits(0x4000);
			map_card_memory(0, 0x4000);
		}
		else // was and remains enabled:
		{
			if (toggled & pageout_armed)
			{
				if (new_config & pageout_armed) set_rom_pageout_bits(0x4000);
				else clear_rom_pageout_bits(0x4000);
			}
			map_card_memory(0, 0x4000);
		}
	}
}


/*	handle rom patch
	-> detect reading from a rom pageout address
	-> detect reading from flash rom while writing in progress
	-> detect reading from rom[0:1] in software ID mode
	returns new opcode
*/
int SmartSDCard::read_memory(int32 cc, uint16 pc, uint8 byte)
{
	Z80* cpu = machine->cpu;

	// reading from a rom pageout address?
	// these bits are only set while the rom page-out mechanism is primed.
	if ((config & pageout_armed) && (pc & 0x0072) == 0x0072)
	{
		clear_rom_pageout_bits();
		if (config & ram_enabled) cpu->unmapWom(0x2000, 8 kB); // unmap ram write page
		if (dip_flash_write_enabled) disable_flash_write();
		prev()->romCS(off);		// enable ZX Spectrum rom
		return *cpu->rdPtr(pc); // handled
	}

	// reading from flash rom while writing in progress?
	// then we are reading from the flash_dummy_page[]
	if (flash_state == flash_writing)
	{
		CoreByte* p = cpu->rdPtr(pc);
		if (p >= flash_dummy_page && p < flash_dummy_page + CPU_PAGESIZE)
		{
			if (cc >= cc_flash_write_end) { finish_flash_write(); }
			else
			{
				byte = flash_byte_written ^ 0x80; // get byte with inverted D7
				flash_byte_written ^= 0x40;		  // toggle D6
				return byte;					  // handled
			}
		}
	}

	// reading from rom[0:1] in software ID mode?
	// while software ID mode active, reading from flash[0|1] returns manufacturer|device ID
	if (flash_software_id_mode && pc <= 1)
	{
		// A0=0: SST Manufacturer Code  = BFH
		// A0=1: SST39SF020 Device Code = B6H
		CoreByte* p = cpu->rdPtr(pc & ~1);
		if (p == rom.getData()) return pc ? 0xB6 : 0xBF; // handled
	}

	return -1;
}

uint8 SmartSDCard::handleRomPatch(uint16 pc, uint8 byte)
{
	if (memory_enabled())
	{
		int result = read_memory(machine->cpu->cpuCycle(), pc, byte);
		if (result >= 0) return result;
	}
	return prev()->handleRomPatch(pc, byte); // not me
}


uint8 SmartSDCard::readMemory(Time t, int32 cc, uint16 addr, uint8 byte)
{
	if (memory_enabled())
	{
		int result = read_memory(cc, addr, byte);
		if (result >= 0) return result;
	}
	return prev()->readMemory(t, cc, addr, byte); // not me
}


/*	write to memory:
	-> detect writing to flash rom
	-> either writing to address $5555 in any memory
	   or writing to any address in flash rom while flash command in progress

	software ID mode: various unusual conditions may be handled wrong
	not handled: overall timing constraints for command sequence (behaviour of real Flash unknown)
				 bail out from command on flash rom reading		 (behaviour of real Flash unknown)
*/
void SmartSDCard::writeMemory(Time t, int32 cc, uint16 addr, uint8 byte)
{
	// test: did this memory write actually hit the Flash Rom?
	CoreByte* a = machine->cpu->wrPtr(addr);
	if (a < flash_dummy_page || a >= flash_dummy_page + CPU_PAGESIZE)
		return prev()->writeMemory(t, cc, addr, byte); // not me

	// write access to flash rom:

	// determine memory address inside flash rom:
	uint   page	   = (config & rompage_bits) << 6;
	uint32 address = (addr & 0x3fff) + page;
	addr		   = address & 0x7FFF; // bits 14:0

	// TODO: test cc
	//	flash_write_cc = cc;

	// starting at current command stage, proced to next stage:
	switch (flash_state)
	{
	case flash_writing:
		if (cc < cc_flash_write_end) return; // ignored
		finish_flash_write();
		// goto fw_nothing

	case flash_idle: // expect 1st byte: write($5555),$AA
		if (byte == 0xF0)
		{
			flash_software_id_mode = no;
			break;
		}											// ok: special case: Software ID exit
		if (addr != 0x5555 || byte != 0xaa) return; // wrong

		machine->cpu->mapDummyWom2Page(0, 0, flash_dummy_page);
		flash_state = flash_AA;
		return; // ok

	case flash_AA:								   // expect 2nd byte: write ($2AAA),$55
		if (addr != 0x2AAA || byte != 0x55) break; // wrong
		flash_state = flash_AA55;
		return; // ok

	case flash_AA55:			   // expect 3rd byte:
		if (addr != 0x5555) break; // error
		if (byte == 0xA0)
		{
			flash_state = flash_AA55A0;
			return;
		} // ok: WRITE BYTE
		if (byte == 0x80)
		{
			flash_state = flash_AA5580;
			return;
		} // ok: chip|sector erase
		if (byte == 0x90)
		{
			flash_software_id_mode = yes;
			break;
		} // ok: Software ID entry
		if (byte == 0xF0)
		{
			flash_software_id_mode = no;
			break;
		}	   // ok: Software ID exit
		break; // error

	case flash_AA55A0: // 4th byte: write byte into flash rom
	{
		rom[address]	   = byte;
		uint max_cc		   = machine->cpu_clock * 30e-6;					  // ~ 105cc
		int	 deviation	   = (int8)(addr ^ byte);							  // ± 128
		cc_flash_write_end = cc + max_cc * (2 * 128 + deviation) / (3 * 128); // 33% .. 99%
		flash_byte_written = byte;
		start_flash_write();
		return;
	}

		// TODO: timing
		/*
			The Program operation, once initiated, will be completed within at most 30 μs. (typ.: 20µs)
			During the Program operation, the only valid reads are Data# Polling and Toggle Bit.
			Any commands written during the internal Program operation will be ignored.

			The software detection includes two status bits: Data# Polling (DQ7) and Toggle Bit (DQ6).
			The end of write detection mode is enabled immediately after writing the byte.

			Data# Polling (DQ7)
			When the SST39SF020 device is in the internal Program operation, any attempt to read DQ7 will produce
			the complement of the true data. Once the Program operation is completed, DQ7 will produce true data.
			During Sector or Chip Erase operation, DQ7 will read ‘0’. When completed, DQ7 will read ‘1’.

			Toggle Bit (DQ6)
			During the internal Program or Erase operation, any consecutive attempts to read DQ6 will produce
			alternating 0’s and 1’s, i.e., toggling between 0 and 1. The Toggle Bit will begin with “1”.
		*/

	case flash_AA5580:							   // expect 4th byte:
		if (addr != 0x5555 || byte != 0xAA) break; // error
		flash_state = flash_AA5580AA;
		return; // ok

	case flash_AA5580AA:						   // expect 5th byte:
		if (addr != 0x2AAA || byte != 0x55) break; // error
		flash_state = flash_AA5580AA55;
		return; // ok

	case flash_AA5580AA55: // 6th byte

		if (addr == 0x5555 && byte == 0x10) // CHIP ERASE ~ 20ms
		{									// erase the entire memory array to the “1’s” state.
			a			= rom.getData();
			CoreByte* e = a + rom.count();
			while (a < e) { *a++ |= 0x000000ff; }
			uint max_cc		   = machine->cpu_clock * 20e-3; // ~ 70000cc
			int	 deviation	   = (int8)(addr ^ byte);		 // ± 128
			cc_flash_write_end = cc + max_cc * (0.8 + deviation * 0.1 / 128);
			flash_byte_written = 0xFF;
			start_flash_write();
			return;
		}

		if (byte == 0x30) // 4K SECTOR ERASE ~ 10ms
		{
			a			= &rom[address & 0x3F000]; // A17:12
			CoreByte* e = a + 4 kB;
			while (a < e) { *a++ |= 0x000000ff; }
			uint max_cc		   = machine->cpu_clock * 10e-3; // ~ 35000cc
			int	 deviation	   = (int8)(addr ^ byte);		 // ± 128
			cc_flash_write_end = cc + max_cc * (0.6 + deviation * 0.2 / 128);
			flash_byte_written = 0xFF;
			start_flash_write();
			return;
		}

		break; // error
	}

	if (!flash_software_id_mode) machine->cpu->unmapWom2(0, 0);
	flash_state = flash_idle;
}


void SmartSDCard::videoFrameEnd(int32 cc) { cc_flash_write_end -= cc; }

// void SmartSDCard::triggerNmi()
//{}


void SmartSDCard::setMemoryEnabled(bool) {}

void SmartSDCard::setForceBankB(bool) {}

void SmartSDCard::enableFlashWrite(bool) {}

void SmartSDCard::setJoystickEnabled(bool) {}


void SmartSDCard::insertJoystick(int id) volatile
{
	if (joystick == joysticks[id]) return;

	if (overlay)
	{
		machine->removeOverlay(overlay);
		overlay = nullptr;
	}
	joystick = joysticks[id];
	if (id != no_joystick) overlay = machine->addOverlay(joystick, "K", gui::Overlay::TopRight);
}
