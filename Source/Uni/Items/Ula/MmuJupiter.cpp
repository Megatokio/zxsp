// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuJupiter.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaJupiter.h"



/*
; MEMORY MAP
;
; $0000 +======================================================+
;       |                                                      |
;       |                   ROM 8K                             |
;       |                                     v $2300          |
; $2000 +======================================================+ - - - - - -
;       |       copy of $2400                 |0|<  cassette  >|
; $2400 +-------------------------------------+-+--------------+
;       |       VIDEO MEMORY 768 bytes        |0| PAD 254 bytes| 1K RAM
; $2800 +-------------------------------------+-+--------------+
;       |       copy of $2c00                 ^ $2700          |
; $2C00 +------------------------------------------------------+
;       |       CHARACTER SET - Write-Only                     | 1K RAM
; $3000 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3400 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3800 +------------------------------------------------------+
;       |       copy of $3c00                                  |
; $3C00 +-------+----------------------------------------------+
;       |SYSVARS| DICT {12} DATA STACK ->         <- RET STACK | 1K RAM
; $4000 +=======+==============================================+ - - - - - -
;       |                                                      |
;                       48K AVAILABLE FOR EXPANSION.
;       |                                                      |
; $FFFF +======================================================+
;
; The 768 bytes of video memory is accessed by the ROM using addresses
; $2400 - $26FF. This gives priority to the video circuitry which also needs
; this information to build the TV picture. The byte at $2700 is set to zero
; so that it is easy for the ROM to detect when it is at the end of the screen.
; The 254 bytes remaining are the PAD - the workspace used by FORTH.
; This same area is used by the tape recorder routines to assemble the tape
; header information but since, for accurate tape timing, the FORTH ROM needs
; priority over the video circuitry, then the ROM uses addresses $2301 - $23FF.
;
; Similarly the Character Set is written to by the ROM (and User) at the 1K
; section starting at $2C00. The video circuitry accesses this using addresses
; $2800 - $2BFF to build the TV picture. It is not possible for the ROM or User
; to read back the information from either address.
*/


MmuJupiter::MmuJupiter ( Machine* m )
:	Mmu( m, isa_MmuJupiter,nullptr,nullptr )
{
	xlogIn("new MmuJupiter");
}


void MmuJupiter::powerOn(int32 cc)
{
	xlogIn("MmuJupiter:init");

	Mmu::powerOn(cc);
	assert( rom.count() == 8 kB );
	assert( ram.count() == 3 kB || ram.count() == 19 kB );

	cpu->mapRom( 0x0000, 0x2000, &rom[0x0000], nullptr, 0 );

	cpu->mapRam( 0x2000, 0x0400, &ram[0x0000], nullptr, 0 );	// vram:  priority:  cpu>crtc
	cpu->mapRam( 0x2400, 0x0400, &ram[0x0000], nullptr, 0 );	// copy:  priority:  crtc>cpu

	cpu->mapWom( 0x2800, 0x0400, &ram[0x0400], nullptr, 0 );	// cram: write-only
	cpu->mapWom( 0x2C00, 0x0400, &ram[0x0400], nullptr, 0 );	// copy

	cpu->mapRam( 0x3000, 0x0400, &ram[0x0800], nullptr, 0 );	// ram
	cpu->mapRam( 0x3400, 0x0400, &ram[0x0800], nullptr, 0 );	// copy
	cpu->mapRam( 0x3800, 0x0400, &ram[0x0800], nullptr, 0 );	// copy
	cpu->mapRam( 0x3C00, 0x0400, &ram[0x0800], nullptr, 0 );	// copy     <-- actually used by forth

	if(ram.count() == 19 kB) cpu->mapRam( 0x4000, 16 kB, &ram[0x0C00], nullptr, 0 );	// ram extension
}













