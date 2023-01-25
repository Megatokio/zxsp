// Copyright (c) 1995 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MmuInves.h"
#include "Machine.h"
#include "Z80/Z80.h"
#include "Ula/UlaInves.h"
#include "ZxInfo.h"
#include "info.h"



/*
from: http://www.zxprojects.com/index.php/the-fix-a-spectrum-blog/29-the-oddities-of-the-inves-spectrum

...

ZXSpectr would have been just another Spectrum emulator, but had a distinctive value added: its author, CÃ©sar HernÃ¡ndez BaÃ±Ã³,
showed the results of his own research about the strange behaviour of the Inves. One of them was the following:

"...It happened that, if you poke'd certain ROM addresses (yes, ROM addresses), the border didn't respond to new BORDER commands,
so the speaker to BEEP commands. After some research, we discovered that pokeing ROM addresses whose least significant byte was 254,
blocked any data from entering the ULA.

To be more precise: if you POKE a value V to addresses 254, 510, 766, ..., 16382; that is, N*256+254 with N ranging from 0 to 63,
that value V acts like an AND mask for any subsequent data directed to the ULA. This means that if V is 0, regardless of the value
issued by the processor, the ULA sees 0 in its data bus. To recover normal operation, 255 has to be "poked" to every "priviledged"
ROM address.

This situation lasted as long as there were power supply to the Spectrum. A hard reset or a RAND USR 0 didn't restore the ULA to
the original state. Only after a power cycle, things would come back to normal..."

...

The following video shows an Inves doing a power cycle to initialize, then a tiny BASIC program is written,
that pokes a 0 to every ROM location from 0 to 63*256+254. After running the program, I do some ULA related tests. After then, I
issue a RAND USR 0, and after that, a hard reset by pushing the reset button. See what happens, and how the Inves can be restored
to its normal state:

<video>

After this last test, I've wondered if this "mask enabling mechanism" only works for memory bus cycles or if it works for I/O bus
cycles too. I've changed the POKE instruction for an OUT instruction in the program showed in the video, and nothing weird has happened.
So, I think that whatever this is, it's a deliberated feature, not an accidental one. The same Microhobby article showed at the
beginning of this post also said that "...it seems that there's some kind of buffer between the Z80 and the peripherals, and some
ports have to be written to, to allow access to the external bus..."

This feature can be used to alter the way other ports work. If I do a:

FOR n=0 TO 63: POKE n*256+p,v: NEXT n

Then, port "p" will be affected by mask "v". That is, any value written to port "p" will be AND'ed with "v". I've tried this with even
ports other than 254 and it works. It seems that port reading operations are not affected, so I cannot test if this works for odd ports
too, as the only even port the Inves has is the Kempston joystick (port 223), which is a read-only port.

To end this article, I want to mention something that seems to have passed unnoticed to the people that has experimented with the Inves:
some people find strange to poke to addresses that are located in ROM area, or at least, it seems so... doesn't it? It might not be that
way. Just recall that the Inves Spectrum+ has 64K of RAM (two 64K x 4bit chips). Couldn't it be that the presumed ROM poke operations
are indeed writting values in RAM? If so, does these RAM values act as configuration values for the ULA or some other parts of the
computer? As a far as I know, there's no schematics for the Inves, and without that, further analysis is not easy.
*/


MmuInves::MmuInves ( Machine* m )
:	MmuZxsp( m, isa_MmuInves,nullptr,nullptr )
{
	xlogIn("new MmuInves");
}
















