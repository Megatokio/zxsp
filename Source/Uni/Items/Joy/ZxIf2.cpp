// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ZxIf2.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "zxsp_helpers.h"

/*
   WoS:
	The 'left' Sinclair joystick maps the joystick directions and the fire button to the 1 (left), 2 (right), 3 (down),
   4 (up) and 5 (fire) keys on the ZX Spectrum keyboard, and can thus be read via port 0xf7fe; see the Port 0xfe section
   for full details. For any of the joystick interfaces which map to keys, any game offering the appropriate form of
   joystick control can instead be played with the listed keys. The 'right' Sinclair joystick maps to keys 6 (left), 7
   (right), 8 (down), 9 (up) and 0 (fire) and can therefore be read via port 0xeffe.

	sinclair1:
		data byte:	%000FUDRL active low
		keys:           54321
		address:	0xF7FE
					%----.0---.----.---0

	sinclair2:
		data byte:	%000LRDUF active low
		keys:           67890
		address:	0xEFFE
					%---0.----.----.---0

	Gemischte Abfrage mit addr = 0xE7FE ?		TODO
	Wert der oberen 3 Datenbits noch nicht geklärt.		TODO
*/


//    WoS:
//  NOTE: Sinclair 1 and Sinclair 2 seem to be exchanged !?!
//
//    #define P_SINCLAIR1                     0xeffe  /* Port address */
//    #define B_SINCLAIR1                     0x1000  /* ---0 ---- ---- ---- */
//    #define P_SINCLAIR2                     0xf7fe  /* Port address */
//    #define B_SINCLAIR2                     0x0800  /* ---- 0--- ---- ---- */


/*	TODO:
	Das ROM ist im MemoryInspector z.Zt. nur mit MemoryInspector::AsSeenByCpu sichtbar.
	evtl. das Rom an Machine.rom anhängen oder anderweitig für MemoryInspector::AllRom und MemoryInspector::RomPages
   sichtbarmachen
*/

#define o_addr nullptr
#define i_addr "----.----.----.---0"


ZxIf2::ZxIf2(Machine* m) : SinclairJoy(m, isa_ZxIf2, external), rom(nullptr), filepath(nullptr) { xlogIn("new ZxIf2"); }


ZxIf2::~ZxIf2() { ejectRom(); }


void ZxIf2::insertRom(cstr path)
{
	assert(isMainThread());
	assert(is_locked());

	ejectRom();

	FD	   fd(path, 'r');
	uint32 sz = uint32(fd.file_size());
	if (sz > 0x4000) sz = 0x4000;
	rom = new Memory(machine, basename_from_path(path), 0x4000);

	read_mem(fd, rom.getData(), sz);
	if (sz <= 0x2000) memcpy(rom.getData() + 0x2000, rom.getData(), 0x2000);

	filepath = newcopy(path);
	prev()->romCS(true);
	machine->cpu->mapRom(0 /*addr*/, 0x4000 /*size*/, rom.getData(), nullptr, 0);
}


void ZxIf2::ejectRom()
{
	assert(isMainThread());
	assert(is_locked());

	prev()->romCS(false);
	delete[] filepath;
	filepath = nullptr;
	rom		 = nullptr;
}


// virtual
void ZxIf2::powerOn(/*t=0*/ int32 cc)
{
	SinclairJoy::powerOn(cc);

	if (rom)
	{
		machine->cpu->mapRom(0 /*addr*/, 0x4000, rom.getData(), nullptr, 0);
		prev()->romCS(true);
	}
}
