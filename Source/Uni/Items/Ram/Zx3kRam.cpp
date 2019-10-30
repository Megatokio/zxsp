/*	Copyright  (c)	Günter Woigk 2012 - 2018
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

#include <QSettings>
#include <QVariant>
#include "Zx3kRam.h"
#include "Machine.h"
#include "Memory.h"
#include "Qt/Settings.h"
#include "Items/Ula/Mmu.h"


// Sinclair ZX 1-3K RAM Memory Extension
// erweitert den ZX80 oder ZX81 um 1, 2 oder 3k
// der TS1000 wurde vermutlich unter Ignorierung seiner eingebauten 2k erweitert
// Effektiv wurde also der Speicher auf 2, 3 oder 4k erweitert.


Zx3kRam::Zx3kRam(Machine*m, uint sz)
:
    ExternalRam(m,isa_Zx3kRam)
{
	xlogIn("new Zx3kRam");

	size = sz ? sz : settings.get_uint(key_zx3k_ramsize,3 kB);    // set in setRamSize()

	machine->ram.grow(1 kB + size);
	machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  ZX80 will most likely crash if not reset
//
Zx3kRam::~Zx3kRam()
{
	xlogIn("~Zx3kRam");

	machine->ram.shrink(machine->model_info->ram_size);
	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
}


// set ram size
// called from Zx3kInsp
// saves size in settings.
// restarts machine
//
void Zx3kRam::setRamSize(uint sz)
{
	assert(is_locked());

	xlogIn("Zx3kRam.setRamSize");

	if(sz==size) return;

	settings.setValue(key_zx3k_ramsize,sz);

	if(sz<size) machine->ram.shrink(1 kB + sz);
	if(sz>size) machine->ram.grow(1 kB + sz);
	size = sz;

	machine->mmu->mapMem();     // map new memory to cpu & to set videoram
	//machine->powerOn();		// initAllItems()
}





