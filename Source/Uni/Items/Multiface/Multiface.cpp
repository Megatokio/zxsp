// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface.h"
#include "Machine.h"


Multiface::Multiface(Machine* m, isa_id id, cstr romfile, cstr o_addr, cstr i_addr) :
	Item(m, id, isa_Multiface, external, o_addr, i_addr),
	rom(m, catstr(isa_names[id], " Rom"), 8 kB),
	ram(m, catstr(isa_names[id], " Ram"), 8 kB),
	nmi_pending(no),
	paged_in(no)
{
	uint8 bu[8 kB];
	FD	  fd(catstr(appl_rsrc_path, romfile));
	fd.read_bytes(bu, 8 kB);
	m->cpu->b2c(bu, rom.getData(), 8 kB);
}


Multiface::~Multiface()
{
	if (paged_in) page_out();
}


void Multiface::powerOn(/*t=0*/ int32 cc)
{
	Item::powerOn(cc);
	nmi_pending = no;
	paged_in	= no;
}

void Multiface::reset(Time t, int32 cc)
{
	Item::reset(t, cc);
	nmi_pending = no;
	if (paged_in) page_out();
}


void Multiface::page_in()
{
	paged_in = yes;
	prev()->romCS(yes);
	machine->cpu->mapRom(0x0000, 8 kB, rom.getData(), nullptr, 0);
	machine->cpu->mapRam(0x2000, 8 kB, ram.getData(), nullptr, 0);
}

void Multiface::page_out()
{
	// wenn wir ausgeblendet werden, dann das Ram aktiv ausblenden,
	// weil das von der MMU nicht gemacht wird. Wir machen das vorher, weil vielleicht
	// dazwischen noch ein Gerät hängt, das da vielleicht selbst Ram einblendet…

	machine->cpu->unmapWom(0x2000, 8 kB);
	paged_in = no;
	prev()->romCS(no);
}
