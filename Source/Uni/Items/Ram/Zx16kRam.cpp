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

#include "Zx16kRam.h"
#include "Machine.h"
#include "Items/Ula/Mmu.h"


/*  Sinclair ZX 16K RAM Memory Extension
    erweitert den ZX81 auf 16k
    Das eingebaute RAM wird deaktiviert

    Note: TS 1500 wurde dadurch auf 32k erweitert!  TODO
*/


//protected
Zx16kRam::Zx16kRam(Machine* m,isa_id id)
:
   ExternalRam(m,id)
{
	xlogIn("new Zx16kRam");

    machine->ram.grow(16*1024);
    machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  c'tor
//  note: BASIC will not use full ram unless reset
//
Zx16kRam::Zx16kRam(Machine*m)
:
    ExternalRam(m,isa_Zx16kRam)
{
	xlogIn("new Zx16kRam");

    machine->ram.grow(16*1024);
    machine->mmu->mapMem();     // map new memory to cpu & set videoram
}


//  d'tor
//  entweder wurde nur das Rampack abgezogen
//  oder die Maschine wurde zerstört.
//  ZX81 will most likely crash if not reset
//
Zx16kRam::~Zx16kRam()
{
	xlogIn("~Zx16kRam");

    machine->ram.shrink(machine->model_info->ram_size);
    machine->mmu->mapMem();     // map new memory to cpu & to set videoram
}




