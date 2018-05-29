/*	Copyright  (c)	Günter Woigk 1995 - 2018
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

#include "MmuZxsp.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaZxsp.h"


/*	creator for use by derived classes:
	for models with mmu
*/
MmuZxsp::MmuZxsp ( Machine* m, isa_id id, cstr o_addr, cstr i_addr )
:
	Mmu( m, id, o_addr, i_addr )
{
	xlogIn("new MmuZxsp");
}


/*	creator for Zxsp mmu:
	for models without mmu
*/
MmuZxsp::MmuZxsp ( Machine* m )
:
	Mmu( m, isa_MmuZxsp,0,0 )
{
	xlogIn("new MmuZxsp");
}


MmuZxsp::~MmuZxsp()
{
	xlogIn("~MmuZxsp");
}


void MmuZxsp::powerOn(int32 cc)
{
	Mmu::powerOn(cc);
	mapMem();
}

void MmuZxsp::mapMem()
{
	assert( ram.count()==16 kB ||  ram.count()==48 kB );
	assert( rom.count()==16 kB || (rom.count()==24 kB && this->isA(isa_MmuTc2068)));
	assert( ula&&cpu );
	assert( ula->isA(isa_UlaZxsp) );

	if(!romdis_in) cpu->mapRom(0x0000,0x4000,&rom[0],NULL,0);

	cpu->mapRam( 0x4000, 0x4000, &ram[0x0000], ula_zxsp->getWaitmap(), ula_zxsp->getWaitmapSize() );

// Note: Das Ausblenden von Ram hier kann Probleme bereiten.
// Cheetah32K erweitert aber machine.ram so dass das hier geht.
// Um es sauber zu machen, müsste man aber wohl in machine.init erst mal allen Speicher unmappen.

	if(ram.count()>0x4000)	cpu->mapRam(0x8000, 0x4000, &ram[0x4000], NULL, 0);
	else					cpu->unmapRam(0x8000, 0x4000);

	if(ram.count()>0x8000)	cpu->mapRam(0xC000, 0x4000, &ram[0x8000], NULL, 0);
	else					cpu->unmapRam(0xC000, 0x4000);

//	UlaZxspPtr(ula)->setVideoRam(ram.Data());	Stört sich mit SPECTRA interface. Sollte hier eh überflüssig sein, da die Ula ja auch ein init() kriegt.
}


/*	ROMCS Eingang wurde aktiviert/deaktiviert
	MMU.ROMCS handles paging of internal ROM.
	=> no need to forward it's own ROMCS state as well.

	In general items should not actively unmap their memory as result of incoming ROMCS.
	Instead they should trust that the sender will map it's rom into the Z80's memory space.
	While their backside ROMCS is active, items must not page in their memory, just store values written to their registers only.

	f=1 -> disable internal rom
*/
void MmuZxsp::romCS( bool f )
{
	if(f==romdis_in) return;   // no change
	romdis_in = f;
	if(f) return;

	cpu->mapRom(0x0000,0x4000,&rom[0],NULL,0);
}


























