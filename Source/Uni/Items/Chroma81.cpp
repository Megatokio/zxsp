// Copyright (c) 2026 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Chroma81.h"
#include "Machine.h"
#include "Ula/Mmu.h"

// TODO:
// audio out is controlled by loss of sync:
// --> use Ula::enableMicOut() !


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
	ula(dynamic_cast<UlaZx81*>(m->ula)),
	dip_rs232_enabled(dip_switches & DipSwitches::EnableRs232),
	dip_ram_at_2000(dip_switches & DipSwitches::Enable8kRamAt2000),
	dip_ram_at_4000(dip_switches & DipSwitches::Enable16kRamAt4000),
	dip_ram_at_C000_and_color_enabled(dip_switches & DipSwitches::Enable16kRamAtC000AndColorModes),
	dip_qs_enabled(dip_switches & DipSwitches::EnableQSCharBoard),
	dip_wrx_enabled(dip_switches & DipSwitches::EnableWRXGraphics),
	color_modes_enabled(no),
	specci_color_mode(no),
	joystick_id(no_joystick),
	ram(new Memory(m, "CHROMA81 ram", 32 kB)),
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
	ram = machine->ram;
	ram->grow(32 kB);

	// set cpu_waitmap in ram because this is used in NMI timing:
	assert(machine->ram[0] & cpu_waitmap); // ZX81 must have, ZX80 doesn't
	for (uint i = 0x0000; i < 0x8000; i++) { ram[i] |= cpu_waitmap; }

	// set cpu_crtc_zx81 flag in ram which can be mapped ≥ 32K:
	for (uint i = 0x4000; i < 0x8000; i++) { ram[i] |= cpu_crtc_zx81; }
}

Chroma81::~Chroma81()
{
	xlogIn("~Chroma81");

	if (machine->crtc == this) machine->setCrtc(ula);
	ejectRom();
	machine->mmu->mapMem(); // map new memory to cpu & to set videoram
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
	// ram[$4000, 8K] at $2000 --> $2000, $A000 and evtl. $E000
	// ram[$4000,16K] at $C000
	// ram[$4400, 1K] at $C400 --> $8400 and evtl. $C400  (QS char board)
	// rom[] at $0000 --> 4/8/16K ?

	// NOTE: we are called after mmu->powerOn() and mmu->reset().

	Z80* cpu = machine->cpu;

	if (dip_ram_at_4000)
	{
		logline("chroma: ram at $4000");
		cpu->mapRam(0x4000, 16 kB, &ram[0], waitmap, waitmap_size);
	}
	else xlogline("chroma: NO ram at $4000");

	// UlaZx81.map_memory() mapped out color ram to $8000 but that does not work:
	cpu->unmapRam(0x8000, 16 kB, waitmap, waitmap_size);

	if (dip_ram_at_C000_and_color_enabled)
	{
		logline("chroma: ram at $c000");
		cpu->mapRam(0xC000, 16 kB, &ram[0x4000], waitmap, waitmap_size);
	}
	else xlogline("chroma: NO ram at $c000");

	if (dip_ram_at_2000) // ram at $2000 and $A000 (and evtl. $E000):
	{
		logline("chroma: ram at $2000");
		cpu->mapRam(0x2000, 8 kB, &ram[0x6000], waitmap, waitmap_size);
		cpu->mapRam(0xA000, 8 kB, &ram[0x6000], waitmap, waitmap_size);
	}
	else xlogline("chroma: NO ram at $2000");

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
	return ula->drawVideoBeamIndicator(cc); //
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

	// if colour modes are NOT enabled:
	// the byte is always read from the internal ROM. (always activated)
	// but in case of the 4k ROM A12 must be 0. (handled by memory mapping in MmuZX80)
	// A13 is probably ignored. (handled by memory mapping in MmuZX80)

	// if QS char board is enabled:
	// if byte would be read from ROM then it is read from QS ram instead. (TODO: address range?)
	// opcode.bit7 is mapped to opcode.bit6 (->A9). ('inverse' bit)

	// if WRX is enabled:
	// the byte can also be read from all mapped chroma ram.

	// if colour modes are enabled:
	// additionally the colour attribute byte is read from screen_addr|$c000 or pixmap_addr|$c000

	ula->run_hsync(cc + 4);
	if (ula->sync == on) return; // SYNC => BLACK!

	bool	  inverted = opcode & 0x80;
	uint	  subaddr  = ((opcode & 0x3F) << 3) + ula->lcntr; // char bitmap address bits A0..A8 (512 bytes)
	CoreByte* p;											  // refresh: ptr to pixels byte in the charmap[]

	if (dip_qs_enabled)
	{
		if (inverted) subaddr += 0x40 << 3; // map bit7 -> bit6 for address
		p = &ram[0x4400];
	}
	else
	{
		Z80* cpu = machine->cpu;
		uint ir	 = cpu->getRegisters().ir;
		//if (inverted && (ir & 0x100)) subaddr += 0x40 << 3; // map bit7 -> bit6 for address

		if (dip_wrx_enabled)
		{
			p = cpu->rdPtr(ir);												  // must be in chroma ram
			if (p < &ram[0] || p > &ram[0x7fff]) p = cpu->rdPtr(ir & 0x3e00); // else in rom
			else subaddr = 0;
		}
		else
		{
			p = cpu->rdPtr(ir & 0x3e00); // ZX81 rom. TODO: cartridge
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
			logline("chroma: out 7fef: colors = %s", ~byte & 0x20 ? "OFF" : byte & 0x10 ? m0 : m1);
			logline("chroma: out 7fef: border = %i", byte & 0x0f);

			color_modes_enabled = byte & 0x20;
			specci_color_mode	= byte & 0x10;
			border_color		= color_modes_enabled ? byte & 0x0F : TVDecoder::white;
			ula->tv_decoder.setBorderColor(cc, border_color);
		}
		else // byte ignored!
		{
			if (byte & 0x20) logline("chroma: out 7fef ignored: switch 6 is OFF!");
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
			logline("chroma: IN $7fef: chroma %s", dip_ram_at_C000_and_color_enabled ? "available" : "NOT available");
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
	dip_qs_enabled = f; //
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
