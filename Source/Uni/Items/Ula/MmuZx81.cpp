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

#include "Mmu.h"
#include "MmuZx81.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaZx81.h"



void MmuZx81::powerOn( /*t=0*/ int32 cc )
{
	MmuZx80::powerOn(cc);
	assert(romdis_in==0);

	uint n = rom.count(); if(n>8 kB) n=8 kB;
	cpu->mapRom(0,n,&rom[0],NULL,0);
}


/*	ROMCS Eingang wurde aktiviert/deaktiviert
	MMU.ROMCS handles paging of internal ROM.
	=> no need to forward it's own ROMCS state as well.

	In general items should not actively unmap their memory as result of incoming ROMCS.
	Instead they should trust that the sender will map it's rom into the Z80's memory space.
	While the ROMCS input is active, items must not page in their memory, just store values written to their registers only.

	ROM_CS:  1->disable
*/
void MmuZx81::romCS( bool f )
{
	xlogline("MmuZx81.romCS(%i)",f);

	if(f==romdis_in) return;
	romdis_in = f;
	if(f) return;		// paged out

	uint n = rom.count(); if(n>8 kB) n=8 kB;
	cpu->mapRom(0,n,&rom[0],NULL,0);
}





















