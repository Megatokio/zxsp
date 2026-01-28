// Copyright (c) 2026 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Chroma81.h"
#include "Machine.h"
#include "Ula/Mmu.h"

// TODO:
// audio out is controlled by loss of sync:
// --> use Ula::enableMicOut() !

/*
Switch 1: ON:  enable 16K RAM at $4000.
Switch 2: ON:  enable WRX graphics support for the Chroma RAM at $2000 and $4000.
		  OFF: CHR$128 support at $2000
Switch 3: ON:  enable 8K RAM at $2000, mirror at $A000, and mirror at $E000 if Sw6=ON.
Switch 4: ON:  enable QS Character Board emulation and 1K ram at $8400, and mirror at $C400 if Sw6=ON.
Switch 5: ON:  enable RS232 socket.
Switch 6: ON:  enable 16K RAM at $C000 and enable the colour modes.
		  OFF: the color control register out($7FEF) is locked to $0F.

_________________________________________________
The CHROMA81 has 32K of RAM.
Any enabled RAM will prevent a device plugged in behind seeing the RAM accesses.

Switch 1: enable 16K RAM at $4000
Switch 3: enable  8K RAM at $2000, mirror at $A000, and mirror at $E000 if Sw6=ON.
Switch 4: enable  1K RAM at $8400,                  and mirror at $C400 if Sw6=ON.
Switch 6: enable 16K RAM at $C000

A cartridge will override ram at $2000 and it's mirror at $A000.

usage:
	$2000	8K:  WRX, CHR$128
	$4000	16K: WRX, CHR$64
	$8400	1K:  QS character board
	$C000	1K:  character colors (mode 0)
	$C000	16K: color attributes (mode 1)

_________________________________________________
CHR$64, CHR$128:
	The charcode is read at CPU PC. Then pixel byte is read in refresh cycle at (IR & $3E00)+charcode*8+lcntr.
	In CHR$64 mode there are only 64 characters and another 64 characters are created by inverting them.
	In CHR$128 mode there are 128 characters. Char 64 to 127 must be stored inverted due to HW.
	I.bit0 selects mode: 0=CHR$64, 1=CHR$128 but only in ram at $2000.
QS board:
	Pixel bytes for 128 chars are read from $8400+charcode*8+lcntr as in CHR$128 mode.
WRX:
	Pixel bytes are read at CPU PC directly.

_________________________________________________
IN $7FEF:
	only bit 5 is valid:
	0 = color modes available (Chroma present and config switch 6 = ON)

_________________________________________________
OUT $7FEF, %00EMIGRB: Select colour mode
	E = enable color
	M = color mode
	IGRB border color

If configuration switch 6 is set to OFF, then the color control register is locked to $0F.
=> black & white and bright white border.

color mode 0:
48K-49K holds a colour look-up table consisting of 128 characters (64+64inverted) each of 8 lines.
=> attr byte is at: $C000+(char*8)+line

attr bytes:	%IGRBigrb
	IGRB paper color
	igrb pen color

color mode 1:
attributes are read from DFILE | 0xC000.  --> 48K-64K
=> DFILE can also be in range $2000 and $8000 but not below $2000!

_________________________________________________
CHR$128 mode:
When RAM is fitted to the internal data bus connected to the ZX81 ROM, then setting the I register to value
between $20-$3F will cause the display mechanism to fetch character pixel patterns from the 8K-16K region.
Normally, there would be 64 characters and then the hardware would automatically generate their inverse forms.
CHR$128 mode extends this to allow the inverse characters to be separately defined.
The pixel pattern lookup address is specified by the I register but only bits 1-7 are used, not bit 0.
CHR$128 therefore notes bit 0 of the I register (which it can do during a Refresh when the IR registers are
placed on the address bus). If bit 0 is 0 then the normal 64 character UDG mode is active.
But if bit 0 is 1 then the extended 128 character UDG mode is active. In 64 character mode, 0.5K  holds
the pixel patterns. In 128 character mode, 1K is used to hold the pixel patterns. The first 0.5K holds
the definitions for the non-inverted characters, and the second 0.5K holds the pixel patterns for the
inverted characters. Bit 7 of the character code normally indicates if a character is to be inverted.
It is noted by Chroma and used to access the first or second 0.5K of RAM point at by the I register.
The display hardware will still use bit 7 of the character code to invert the pixel patterns, so the pixel
patterns for the inverse characters need to be stored in RAM in inverted form.

For the standard lo-res display, the display hardware fetches the character code via an opcode fetch
and then during the Refresh cycle of that opcode fetch, the pixel pattern corresponding to the character code
is looked up from the ROM. During a Refresh, the IR registers are placed on the address bus. For CHR$128 mode,
the I register (bits1-7) is combined with the 6-bit character code and the 3-bit line counter to form the
look up address to read the pixel pattern from. For WRX mode, the IR registers are used to fetch the actual
pixel pattern byte from RAM to display.
If WRX mode is enabled (via switch 2) then WRX mode applies to the ram at $4000 and at $2000 (if switch 3 is on).
Chroma’s WRX switch 2 only affects the RAM provided by Chroma; other RAM is not affected.

_________________________________________________
Sound:
The video signal is output to the audio channels of the SCART socket whenever video synchronisation is lost.

_________________________________________________
Rear expansion bus:
the Chroma81 suppresses memory access behind itself.
With a ROM cartridge plugged in, the /MREQ and /RFSH lines of the rear expansion bus is held high to prevent
a device plugged in behind seeing ROM address accesses.

_________________________________________________
Reset:
A reset button is directly connected to /RST.
resetting the CPLD takes somewhat longer than CPU reset.

_________________________________________________
Cartridge socket:
With a ROM cartridge plugged in, the /MREQ and /RFSH lines of the rear expansion bus is held high to prevent
a device plugged in behind seeing ROM address accesses.
The ROM cartridge socket also overrides the character bitmap generator inside the ZX81.
A ROM cartridge can populate all of the ROM address space from $0000-$3FFF.
A ROM cartridge will override the onboard 8K RAM at $2000 and it's mirror at $A000 if it is enabled.
NIMP: Early ZX81 ROMS had a bug in the FP math. One fix was a HW which forced some bits at certain addresses.

_________________________________________________
Joystick socket:
The joystick socket supports the cursor format.
The socket is read using two input ports: $F7FE and $EFFE.
The data matches keyboard key 5..8 + 0.

_________________________________________________
RS232 Socket:
The RS232 socket is enabled using configuration switch 5.
Handshake: For a PC, the RTS line should be used, whereas for a serial printer the DTR line should be used.

The RS232 socket is controlled by I/O port $FEEF. The address lines are fully decoded.
The data is sent and received INVERTED.
Upon resetting the ZX81 or disabling of the RS232 socket via switch 5, the output lines are set to the IDLE state.

OUT $FEEF, %000000HD
	H = handshake: 1=ok to send
	D = data, inverted, 0=idle

%DHxxxxx0 = IN $FEEF
	D = data, inverted, 0=idle
	H = handshake, 1=ok to send
	0 = RS232 port present & enabled
*/


namespace zxsp
{

// OUT $7FEF %0111.1111.1110.1111: %00EMIGRB: Select colour mode
// OUT $FEEF %1111.1110.1110.1111: %000000HD: RS232

// IN  $7FEF %0111.1111.1110.1111: %xCxxxxxx: only bit 6 is valid: 0 = color modes available
// IN  $FEEF %1111.1110.1110.1111: %DHxxxxx0: RS232
// IN  $F7FE %----.0---.----.---0  and
// IN  $EFFE %---0.----.----.---0: The joystick is mapped to keys 5,6,7,8 and 0

constexpr char o_addr[] = "-111.111-.1110.1111";
constexpr char i_addr[] = "----.----.----.----";

Chroma81::Chroma81(Machine* m, uint dip_switches) :
	Crtc(m, isa_Chroma81, isa_Crtc, external, o_addr, i_addr),
	ula(dynamic_cast<UlaZx81*>(machine->ula)),
	dip_rs232_enabled(dip_switches & DipSwitches::EnableRs232),
	dip_ram_at_2000(dip_switches & DipSwitches::Enable8kRamAt2000),
	dip_ram_at_4000(dip_switches & DipSwitches::Enable16kRamAt4000),
	dip_ram_at_C000_and_color_enabled(dip_switches & DipSwitches::Enable16kRamAtC000AndColorModes),
	dip_qs_enabled(dip_switches & DipSwitches::EnableQSCharBoard),
	dip_wrx_enabled(dip_switches & DipSwitches::EnableWRXGraphics),
	color_modes_enabled(no),
	specci_color_mode(no),
	joystick_id(no_joystick),
	//ram(new Memory(m, "CHROMA81 ram", 32 kB)),
	ram(machine->ram),
	rom(nullptr),
	waitmap(ula->waitmap),
	waitmap_size(ula->waitmap_size),
	rom_filepath(nullptr)
{
	assert(machine->isA(isa_MachineZx81));
	assert(ula);
	screen = ula->screen;
	assert(screen);
	machine->setCrtc(this);

	//TODO
	ram->grow(32 kB);

	// set cpu_waitmap in ram because this is used in NMI timing:
	assert(ram[0] & cpu_waitmap); // ZX81 must have, ZX80 doesn't
	for (uint i = 0x0000; i < 0x8000; i++) { ram[i] |= cpu_waitmap; }

	// set cpu_crtc_zx81 flag in ram which can be mapped ≥ 32K:
	for (uint i = 0x4000; i < 0x8000; i++) { ram[i] |= cpu_crtc_zx81; }
}

Chroma81::~Chroma81()
{
	xlogIn("~Chroma81");

	if (machine->crtc == this) machine->setCrtc(ula);
	ejectRom();
	if (ram.count() == 32 kB) ram.shrink(16 kB); // TODO
	machine->mmu->mapMem();						 // map new memory to cpu
}

void Chroma81::powerOn(/*t=0*/ int32 cc)
{
	xlogIn("Chroma81:powerOn");
	assert(screen);

	machine->setCrtc(this);
	border_color		= TVDecoder::white; // should be set in Crtc::powerOn()
	color_modes_enabled = no;
	Crtc::powerOn(cc);
	map_memory();
}

void Chroma81::reset(Time t, int32 cc)
{
	// the Chroma CPLD takes 1sec longer to reset
	// this is not replicated

	xlogIn("Chroma81::reset");

	border_color		= TVDecoder::white; // should be set in Crtc::reset()
	color_modes_enabled = no;
	Crtc::reset(t, cc);
	map_memory();
}

void Chroma81::map_memory()
{
	// ram[$0000,16K] at $4000
	// ram[$4000,16K] at $C000
	// ram[$6000, 8K] at $2000, $A000
	// ram[$4400, 1K] at $8400
	// rom[] at $0000 --> 4/8/16K ?

	// NOTE: we are called after mmu->powerOn() and mmu->reset().

	Z80* cpu = machine->cpu;

	if (dip_ram_at_4000)
	{
		xlogline("Chroma81: ram at $4000");
		cpu->mapRam(0x4000, 16 kB, &ram[0], waitmap, waitmap_size);
	}
	else xlogline("Chroma81: NO ram at $4000");

	// UlaZx81.map_memory() mapped ram[$4000] to address $8000 but that does not work:
	cpu->unmapRam(0x8000, 16 kB, waitmap, waitmap_size);

	if (dip_ram_at_C000_and_color_enabled)
	{
		xlogline("Chroma81: ram at $c000");
		cpu->mapRam(0xC000, 16 kB, &ram[0x4000], waitmap, waitmap_size);
	}
	else xlogline("Chroma81: NO ram at $c000");

	if (dip_ram_at_2000) // ram at $2000 and $A000 (and evtl. $E000):
	{
		xlogline("Chroma81: ram at $2000");
		cpu->mapRam(0x2000, 8 kB, &ram[0x6000], waitmap, waitmap_size);
		cpu->mapRam(0xA000, 8 kB, &ram[0x6000], waitmap, waitmap_size);
	}
	else xlogline("Chroma81: NO ram at $2000");

	if (dip_qs_enabled) // ram at $8400 (and evtl. $C400):
	{
		cpu->mapRam(0x8400, 1 kB, &ram[0x4400], waitmap, waitmap_size);
	}

	if (rom)
	{
		// overrides ram at $2000 and evtl. $A000
		// TODO
	}
}

int32 Chroma81::updateScreenUpToCycle(int32 cc)
{
	return ula->updateScreenUpToCycle(cc); //
}

int32 Chroma81::doFrameFlyback(int32 cc)
{
	return ula->doFrameFlyback(cc); //
}

void Chroma81::drawVideoBeamIndicator(int32 cc)
{
	ula->drawVideoBeamIndicator(cc); //
}

void Chroma81::setBorderColor(uint8) { TODO(); }

void Chroma81::crtcRead(int32 cc, uint pc, uint opcode)
{
	// an instruction was read at an address with A15=1 and returned an opcode with A6=0
	// => the Ula reads a video byte and fakes a NOP for the CPU
	//	  the NOP is already handled in the Z80 macro

	// bits D0 … D5 of the opcode are used as character code.
	// D7 is used to complement the output.

	// the screen byte is read from an address composed as following:
	// A0…2 = LCNTR
	// A3…A8 = D0…D5 from opcode
	// A9…A15 = A1…A7 from I register

	// normally A14+A15 are ignored and the byte is always read from the internal ROM. (lower 16K)

	// if QS char board is enabled:
	// the byte is always read from QS ram at $8400.
	// opcode.bit7 is mapped to opcode.bit6 (->A9). ('inverse' bit)

	// if RAM is enabled at $4000 then the byte can also be read from $4000.
	// if WRX is OFF then bytes read from $2000 use CHR$128 mode if I.bit0=1.

	// if WRX is ON:
	// the byte read at $2000 or $4000 is directly used as pixels.

	// if colour modes are enabled:
	// additionally the colour attribute byte is read from screen_addr|$c000 or pixmap_addr|$c000

	ula->run_hsync(cc + 4);
	if (ula->sync == on) return; // SYNC => BLACK!

	bool	  inverted = opcode & 0x80;
	uint	  subaddr  = ((opcode & 0x3F) << 3) + ula->lcntr; // char bitmap address bits A0..A8 (512 bytes)
	CoreByte* p;											  // refresh: ptr to pixels byte in the charmap[]

	if (dip_qs_enabled)
	{
		if (inverted) subaddr += 64 * 8;
		p = &ram[0x4400];
	}
	else
	{
		Z80* cpu = machine->cpu;
		uint ir	 = cpu->getRegisters().ir;

		if (dip_wrx_enabled)
		{
			// WRX only in Chroma ram at $2000 and $4000:
			if (ir >= 0x8000) goto dflt;
			p = cpu->rdPtr(ir);
			if (p < &ram[0] || p > &ram[0x7fff]) goto dflt; // not in Chroma ram
			subaddr = 0;
		}
		else // WRX off
		{
			// WRX off: ram at $2000 supports CHR$128 mode:
			if (inverted && (ir & 0xE100) == 0x2100 && dip_ram_at_2000) subaddr += 64 * 8;
		dflt:
			// TODO: cartridge overrides lower 16K and mirror of $2000 at $A000
			p = cpu->rdPtr(ir & 0x3e00);
		}
	}

	uint8 pixels = uint8(p[subaddr]);
	uint8 attr	 = TVDecoder::black_on_white;
	if (color_modes_enabled)
	{
		attr = specci_color_mode ?				 //
				   ram[0x4000 + (pc & 0x3fff)] : // color from attr at (screenbyte_address | $C000)
				   ram[0x4000 + subaddr];		 // color from (charbitmap_subaddress | $C000)
	}

	ula->tv_decoder.storePixelByte(cc + 4, inverted ? ~pixels : pixels, attr);
}

void Chroma81::output(Time, int32 cc, uint16 addr, uint8 byte)
{
	// OUT $7FEF %0111.1111.1110.1111: %00EMIGRB: Select colour mode
	// OUT $FEEF %1111.1110.1110.1111: %000000HD: RS232

	if (addr == 0x7fef) // select color mode
	{
		if (dip_ram_at_C000_and_color_enabled)
		{
			static constexpr char m0[] = "colorized characters";
			static constexpr char m1[] = "specci screen attributes";
			xlogline("chroma: out 7fef: colors = %s", ~byte & 0x20 ? "OFF" : byte & 0x10 ? m0 : m1);
			xlogline("chroma: out 7fef: border = %i", byte & 0x0f);

			color_modes_enabled = byte & 0x20;
			specci_color_mode	= byte & 0x10;
			border_color		= color_modes_enabled ? byte & 0x0F : TVDecoder::white;
			ula->tv_decoder.setBorderColor(cc, border_color);
		}
		else // byte ignored!
		{
			if (byte & 0x20) xlogline("chroma: out 7fef ignored: switch 6 is OFF!");
		}
	}
	else if (dip_rs232_enabled && addr == 0xfeef)
	{
		// TODO
	}
}

// read joystick:
// 'get' also flags the joystick as 'active' which allows the keyboard joystick to snatch the keys.
// this does not work for the Cursor joystich as with the Sinclair joysticks as they always look 'active'.
uint8 Chroma81::getJoystickButtonsFUDLR() { return machine->getJoystickButtons(joystick_id); }
uint8 Chroma81::peekJoystickButtonsFUDLR() const volatile { return machine->peekJoystickButtons(joystick_id); }

void Chroma81::input(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask)
{
	if (addr & 0x0001) // bit0 = 1 --> chroma or rs232
	{
		// IN  $7FEF %0111.1111.1110.1111: %xCxxxxxx: only bit 5 is valid: 0 = color modes available
		// IN  $FEEF %1111.1110.1110.1111: %DHxxxxx0: RS232

		if (addr == 0x7fef) // chroma colors available?
		{
			xlogline("chroma: IN $7fef: chroma %s", dip_ram_at_C000_and_color_enabled ? "available" : "NOT available");
			if (dip_ram_at_C000_and_color_enabled) byte &= 0b11011111;
			mask |= 0b00100000;
			return;
		}
		else if (dip_rs232_enabled && addr == 0xfeef) // rs232 input
		{
			return; // TODO
		}
	}
	else // bit0 = 0 --> keyboard / joystick
	{
		// IN  $F7FE %1111.0111.1111.1110  and
		// IN  $EFFE %1110.1111.1111.1110: The joystick is mapped to keys 5,6,7,8 and 0

		if (~addr & 0x1800) // only 0-bits are decoded (as for keyboard)
		{
			if (uint8 state = getJoystickButtonsFUDLR())
			{
				// cursor keys:
				//   left  -> key "5" -> bit 4   port 0xf7fe
				//   down  -> key "6" -> bit 4   port 0xeffe
				//   up    -> key "7" -> bit 3   port 0xeffe
				//   right -> key "8" -> bit 2   port 0xeffe
				//   fire  -> key "0" -> bit 0   port 0xeffe

				uint8 mybyte = 0;

				if (~addr & 0x0800) // 0xf7fe
				{
					mybyte |= (state & 2) << 3; // left -> key "5" -> bit 4
				}

				if (~addr & 0x1000) // 0xeffe
				{
					mybyte |= ((state & 1) << 2)	 // right
							  + ((state & 4) << 2)	 // down
							  + ((state & 8) << 0)	 // up
							  + ((state & 16) >> 4); // fire
				}

				byte &= ~mybyte; // oK => only 0-bits make it to the bus
				mask |= mybyte;
			}
		}
	}
}

void Chroma81::setRS232Enabled(bool f)
{
	if (f) logline("Chroma81:setRS232Enabled(): TODO");
}

void Chroma81::setQSEnabled(bool f)
{
	dip_qs_enabled = f;
	machine->mmu->mapMem();
	map_memory();
}

void Chroma81::setWRXEnabled(bool f)
{
	dip_wrx_enabled = f; //
}

void Chroma81::set8kRamAt2000Enabled(bool f)
{
	dip_ram_at_2000 = f;
	machine->mmu->mapMem();
	map_memory();
}

void Chroma81::set16kRamAt4000Enabled(bool f)
{
	dip_ram_at_4000 = f;
	machine->mmu->mapMem();
	map_memory();
}

void Chroma81::setRamAtC000AndColorEnabled(bool f)
{
	dip_ram_at_C000_and_color_enabled = f;

	if (f == off)
	{
		color_modes_enabled = false;
		border_color		= TVDecoder::white;
		ula->tv_decoder.setBorderColor(machine->current_cc(), border_color);
	}

	machine->mmu->mapMem();
	map_memory();
}

void Chroma81::insertRom(cstr path) { logline("Chroma81::insertRom(): TODO"); }
void Chroma81::ejectRom() { logline("Chroma81::ejectRom(): TODO"); }

} // namespace zxsp


/*








































*/
