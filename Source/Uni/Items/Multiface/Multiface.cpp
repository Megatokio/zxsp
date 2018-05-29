/*	Copyright  (c)	Günter Woigk 2015 - 2018
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
*/

#include "Multiface.h"
#include "Machine.h"


Multiface::Multiface(Machine* m, isa_id id, cstr romfile, cstr o_addr, cstr i_addr)
:
	Item(m,id,isa_Multiface,external,o_addr,i_addr),
    rom(m,catstr(isa_names[id]," Rom"), 8 kB),
    ram(m,catstr(isa_names[id]," Ram"), 8 kB),
	nmi_pending(no),
	paged_in(no)
{
	uint8 bu[8 kB];
	FD fd(catstr(appl_rsrc_path,romfile));
	fd.read_bytes(bu,8 kB);
	m->cpu->b2c(bu,rom.getData(),8 kB);
}


Multiface::~Multiface()
{
	if(paged_in) page_out();
}


void Multiface::powerOn( /*t=0*/ int32 cc )
{
	Item::powerOn(cc);
	nmi_pending = no;
	paged_in = no;
}

void Multiface::reset( Time t, int32 cc )
{
	Item::reset(t,cc);
	nmi_pending = no;
	if(paged_in) page_out();
}


void Multiface::page_in()
{
	paged_in = yes;
	prev()->romCS(yes);
	machine->cpu->mapRom(0x0000,8 kB, rom.getData(), NULL,0);
	machine->cpu->mapRam(0x2000,8 kB, ram.getData(), NULL,0);
}

void Multiface::page_out()
{
	// wenn wir ausgeblendet werden, dann das Ram aktiv ausblenden,
	// weil das von der MMU nicht gemacht wird. Wir machen das vorher, weil vielleicht
	// dazwischen noch ein Gerät hängt, das da vielleicht selbst Ram einblendet…

	machine->cpu->unmapWom(0x2000,8 kB);
	paged_in = no;
	prev()->romCS(no);
}


void Multiface::saveToFile ( FD& ) const noexcept(false) /*file_error,bad_alloc*/
{
	TODO();
}

void Multiface::loadFromFile( FD& ) noexcept(false) /*file_error,bad_alloc*/
{
	TODO();
}







