// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MmuTk85.h"
#include "Mmu.h"


/*
	the TK 85 haven't a PSG. But it came prepared to allocate one -
	There is a space for the same PSG from the MSX (The AY),
	and there is a memory adress that is never used - This adress can be used to control the PSG.
*/


MmuTk85::MmuTk85(Machine* m) : MmuZx81(m, isa_MmuTk85) {}
