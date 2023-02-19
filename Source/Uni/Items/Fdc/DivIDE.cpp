// Copyright (c) 2014 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DivIDE.h"
#include "IdeDevice.h"
#include "Machine/Machine.h"
#include "Z80/Z80.h"


/*	TODO:
	cpu patch tested in every M1 cycle -> BC, ED etc.
	emulation of writing to eeprom
*/


#define o_addr "---- ---- 1-1- --11"
#define i_addr "---- ---- 101- --11"

static constexpr uint control_register_address = 0b11100011;
static constexpr uint ide_register_address	   = 0b10100011;
static constexpr uint register_bits_mask	   = 0b00011100; // bits for IDE register address in i/o address
static constexpr uint address_bits_mask		   = 0b11100011;

static constexpr uint DivIDE_bit_mask = 1 << 6; // bit in address selects between DivIDE ctl reg (1) and IDE reg (0)
static constexpr uint MAPRAM_bit_mask = 1 << 6; // bit in DivIDE ctl reg
static constexpr uint CONMEM_bit_mask = 1 << 7; // bit in DivIDE ctl reg


DivIDE::DivIDE(Machine* machine, uint ramsize, cstr romfile) :
	MassStorage(machine, isa_DivIDE, external, o_addr, i_addr),
	rom(machine, "DivIDE Rom", 8 kB),
	ram(machine, "DivIDE Ram", ramsize == 16 kB ? 16 kB : 32 kB),
	cf_card(nullptr),
	ide_data_latch_state(random() & 1),
	control_register(0),						 // All bits are reset to '0' after power-on.
	jumper_E(),									 // set by insertRom()
	jumper_A(machine->isA(isa_MachineZxPlus2a)), // for most models off, for +2A/+3 it must be set
	auto_paged_in(0),							 // state of auto-paging
	own_romdis_state(0),						 // own state
	romfilepath(nullptr)
{
	xlogIn("new DivIDE");
	insertRom(romfile);
}

DivIDE::~DivIDE()
{
	if (own_romdis_state &&
		romdis_in == no) // this should always be true. else romCS chain is left in inconsistent state
	{
		control_register = 0;  // clear conmem bit
		auto_paged_in	 = no; // !conmem && !autopaged => will unmap all
		mapMemory();		   // unmap our memory => also unmaps ram at 0..16k!
	}

	delete cf_card;
	delete[] romfilepath;
}

void DivIDE::powerOn(int32 cc)
{
	MassStorage::powerOn(cc);
	assert(romdis_in == off);

	ide_data_latch_state = random() & 1;
	control_register	 = 0;
	auto_paged_in		 = off; // the FF that is toggled by the auto-paging addresses
	own_romdis_state	 = off; // own state = auto-state + CONMEM
	applyRomPatches();
	for (uint i = 0; i < ram.count(); i++) ram[i] |= random() & 0xff;
	if (cf_card) cf_card->reset(0.0);
}

void DivIDE::romCS(bool f)
{
	// Handle change at rearside ROMCS input
	// Note: DivIDE 57c has no rearside port!

	if (f == romdis_in) return;
	romdis_in = f;

	if (!own_romdis_state) // our memory is not paged in.
	{					   // => we are not involved in memory paging.
		prev()->romCS(f);  // => just pass the bucket.
		return;
	}

	// own_romdis_state = true!

	if (f) // paged out!
	{
		// prev()->romCS(1);	since we are paged in this is a nop

		// no need to unmap our readable memory: caller will map it's own rom
		// but we probably must actively unmap our writable memory:

		CoreByte* roma = rom.getData();
		CoreByte* rama = ram.getData();
		CoreByte* rame = rama + ram.count();

		CoreByte* p = machine->cpu->wrPtr(0);
		if (p == roma || (p >= rama && p < rame)) machine->cpu->unmapWom(0x0000 /*addr*/, 8 kB /*size*/);

		p = machine->cpu->rdPtr(0x2000);
		if (p >= rama && p < rame) machine->cpu->unmapWom(0x2000 /*addr*/, 8 kB /*size*/);
	}
	else // paged in!
	{
		mapMemory(); // also emits romCS()
	}
}

void DivIDE::applyRomPatches()
{
	// apply rom patches, regardless whether paging is enabled or not:
	{ // enable-hooks go into the built-in rom:
		MemoryPtr rom =
			machine->rom; // TODO: sollten vor uns noch andere roms sein, müssten die enable-hooks auch da rein...

		uint pagesize = machine->model_info->page_size;
		assert(pagesize == 0x4000 || pagesize == 0x2000);

		for (uint page = 0; page < rom.count(); page += pagesize)
		{
			rom[page + 0x000] |= cpu_patch;														   // RESET
			rom[page + 0x008] |= cpu_patch;														   // ERROR1
			rom[page + 0x038] |= cpu_patch;														   // INT IM1
			rom[page + 0x066] |= cpu_patch;														   // NMI
			rom[page + 0x4C6] |= cpu_patch;														   // SAVE
			rom[page + 0x562] |= cpu_patch;														   // LOAD
			for (uint i = 0x3d00; i <= 0x3dff; i++) rom[page + (i & (pagesize - 1))] |= cpu_patch; // TR-DOS
		}
	}

	// disable hooks go into the DivIDE rom:
	// TODO: sollten hinter uns noch roms sein, müssen die disable hooks auch da rein...
	for (uint i = 0x1ff8; i <= 0x1fff; i++) rom[i] |= cpu_patch; // 'off-area'
}

void DivIDE::reset(Time t, int32 cc)
{
	// push reset:
	// /reset is not connected to anything on the DivIDE 57c.
	// it is connected to IDE /reset.

	MassStorage::reset(t, cc);
	if (cf_card) cf_card->reset(t);
}

void DivIDE::mapMemory()
{
	// TODO: evtl. alten state cachen und vergleichen ob überhaupt was getan werden muss

	own_romdis_state = conmem_is_set() || auto_paged_in;

	if (romdis_in) return; // wir sind von hinten übersteuert

	prev()->romCS(own_romdis_state);

	if (own_romdis_state) // on:
	{
		// map ram at $2000:
		{
			int rampage = mapped_rampage();
			if (rampage != 3 || conmem_is_set() || !mapram_is_set()) // ram writable
			{
				machine->cpu->mapRam(0x2000 /*addr*/, 8 kB /*size*/, &ram[rampage << 13], nullptr, 0);
			}
			else // ram page #3 write protected
			{
				machine->cpu->mapRom(0x2000 /*addr*/, 8 kB /*size*/, &ram[3 << 13], nullptr, 0);
				machine->cpu->unmapWom(0x2000 /*addr*/, 8 kB /*size*/, nullptr, 0);
			}
		}

		// map ram, rom or nothing at $0000:

		if (mapram_is_set() && !conmem_is_set()) // write protected ram#3 at 0x0000
		{
			machine->cpu->mapRom(0, 8 kB, &ram[3 << 13], nullptr, 0);
			machine->cpu->unmapWom(0 /*addr*/, 8 kB /*size*/, nullptr, 0);
		}
		else if (conmem_is_set() && !jumper_E) // rom writable
		{
			machine->cpu->mapRam(0 /*addr*/, 8 kB /*size*/, &rom[0], nullptr, 0);
		}
		else // rom write protected
		{
			machine->cpu->mapRom(0 /*addr*/, 8 kB /*size*/, &rom[0], nullptr, 0);
			machine->cpu->unmapWom(0 /*addr*/, 8 kB /*size*/, nullptr, 0);
		}
	}

	else // off:
	{	 // the mmu will already have unmapped our readable memory
		 // but we still need to unmap our writable memory:

		CoreByte* roma = rom.getData();
		CoreByte* rama = ram.getData();
		CoreByte* rame = rama + ram.count();

		CoreByte* p = machine->cpu->wrPtr(0);
		if (p == roma || (p >= rama && p < rame)) machine->cpu->unmapWom(0x0000 /*addr*/, 8 kB /*size*/);

		p = machine->cpu->rdPtr(0x2000);
		if (p >= rama && p < rame) machine->cpu->unmapWom(0x2000 /*addr*/, 8 kB /*size*/);
	}
}

void DivIDE::input(Time t, int32 /*cc*/, uint16 addr, uint8& byte, uint8& mask)
{
	// input from IDE register
	// the DivIDE control register can't be read
	// so any input must be for the IDE registers
	// NOTE: if only 1 drive (master) is attached (which is true for the current implementation)
	//       then it must respond as if selected, except for reading register 7. (ATA5 pg.288)

	assert((addr & address_bits_mask) == ide_register_address);

	mask = 0xff; // data bus is always driven, either by xcver 245 or by latch 573

	if (addr & register_bits_mask) // not the data register
	{
		ide_data_latch_state = 0; // reset the odd/even FF
		if (cf_card) byte &= cf_card->readRegister(t, addr >> 2);
		xlogline("DivIDE.in: idereg %i = %2x", (addr >> 2) & 7, uint(byte));
	}
	else // data register
	{
		ide_data_latch_state ^= 1; // toggle the odd/even FF

		if (ide_data_latch_state) // first ("odd") access? => read low byte, store high byte
		{
			uint16 n = 0xffff;
			if (cf_card) n = cf_card->readData(t);
			ide_data_in_latch = n >> 8;
			byte &= n;
		}
		else // second ("even") access? => return stored high byte
		{
			byte &= ide_data_in_latch;
		}
		xlogline("DivIDE.in: idereg %i = %2x", (addr >> 2) & 7, uint(byte));
	}
}

void DivIDE::output(Time t, int32 /*cc*/, uint16 addr, uint8 byte)
{
	// output to DivIDE control register or IDE register

	if (addr & DivIDE_bit_mask) // DivIDE control register
	{
		assert((addr & address_bits_mask) == control_register_address);

		if (addr & register_bits_mask) return; // void address: all 8 bits decoded: 0b11100011

		xlogline("DivIDE.out: ctlreg = %2x", uint(byte));

		ide_data_latch_state = 0;

		byte |= control_register & MAPRAM_bit_mask; // MAPRAM can't be reset

		uint x			 = control_register ^ byte;
		control_register = byte;

		if (x & CONMEM_bit_mask) // CONMEM toggled?
		{
			mapMemory();
		}
		else if (x & 0x7f) // MAPRAM toggled (can only be set) or selected ram page toggled?
		{
			if (own_romdis_state) mapMemory(); // if currently mapped in => update mapping
		}
	}
	else // IDE register
	{
		assert((addr & address_bits_mask) == ide_register_address);

		xlogline("DivIDE.out: idereg %i = %2x", (addr >> 2) & 7, uint(byte));

		if (addr & register_bits_mask) // not the data register
		{
			ide_data_latch_state = 0;								 // reset the odd/even FF
			if (cf_card) cf_card->writeRegister(t, addr >> 2, byte); // write register goes to both, to master & slave
		}
		else // data register
		{
			ide_data_latch_state ^= 1; // toggle the odd/even FF

			if (ide_data_latch_state) // first ("odd") access? => store low byte
			{
				ide_data_out_latch = byte;
			}
			else // second ("even") access? => write stored low byte + high byte
			{
				if (cf_card && cf_card->isSelected()) cf_card->writeData(t, ide_data_out_latch + byte * 256);
			}
		}
	}
}

uint8 DivIDE::handleRomPatch(uint16 pc, uint8 opcode)
{
	// Handle Rom patch.
	// Here we control the auto-paging.

	xlogline("DivIDE::handleRomPatch(pc=%i)", int(pc));

	if (jumper_E) // auto-paging enabled
	{
		// auto page out:
		if ((pc | 7) == 0x1fff)
		{
			xlogline("DivIDE: page OUT");
			if (auto_paged_in)
			{
				auto_paged_in = off;
				if (!conmem_is_set()) mapMemory();
			}
			return opcode; // don't re-read opcode
		}

		// auto page in after opcode:
		if (pc == 0 || pc == 8 || pc == 0x38 || pc == 0x66 || pc == 0x4c6 || pc == 0x562)
		{
			xlogline("DivIDE: page IN");
			if (!auto_paged_in)
			{
				auto_paged_in = on;
				if (!conmem_is_set()) mapMemory();
			}
			return opcode; // don't re-read opcode
		}

		// auto page in instantly:
		if ((pc | 0xff) == 0x3dff)
		{
			xlogline("DivIDE: page IN (instantly)");
			if (!auto_paged_in)
			{
				auto_paged_in = on;
				if (!conmem_is_set()) mapMemory();
			}
			return machine->cpu->peek(pc); // re-read opcode
		}
	}

	return prev()->handleRomPatch(pc, opcode);
}

void DivIDE::setJumperE(bool f)
{
	// insert or remove jumper_E:
	// jumper set:	enable automatic memory paging by executing trigger addresses in Rom
	//				write-protect Rom

	if (jumper_E == f) return;
	jumper_E = f;

	auto_paged_in = off; // reset paging FF

	mapMemory(); // update paging and rom write protection state
}

void DivIDE::saveRom(FD& fd)
{
	assert(rom.count() == 8 kB);
	write_mem(fd, rom.getData(), 8 kB);

	//	settings.setValue(key_divide_rom_file,fd.filepath());
	//	delete[] romfilepath;
	//	romfilepath = newcopy(path);
}

cstr DivIDE::insertRom(cstr path)
{
	assert(rom.count() == 8 kB);

	if (!path) path = catstr(appl_rsrc_path, default_rom_path);

	try
	{
		FD fd(path, 'r'); // throws

		off_t sz = fd.file_size();
		if (sz != 8 kB) return usingstr("Rom size is 8 kByte, but file size is %lu", sz);

		read_mem(fd, rom.getData(), 8 kB); // throws
		delete[] romfilepath;
		romfilepath = newcopy(path);
		applyRomPatches();
		setJumperE(true);
		return nullptr; // ok
	}
	catch (FileError& e)
	{
		delete[] romfilepath;
		romfilepath = nullptr;
		setJumperE(false);
		return e.what();
	}
}

void DivIDE::ejectDisk()
{
	delete cf_card;
	cf_card = nullptr;
}

void DivIDE::insertDisk(cstr path)
{
	// note: IdeCFCard shows alerts on error

	if (cf_card) ejectDisk();
	cf_card = new IdeCFCard(path);		   // master
	if (!cf_card->isLoaded()) ejectDisk(); // open file failed!
}

void DivIDE::audioBufferEnd(Time t)
{
	if (cf_card) cf_card->audioBufferEnd(t);
}

cstr DivIDE::getDiskFilename() const volatile
{
	assert(isMainThread());

	if (!cf_card) return nullptr;
	cstr fpath = cf_card->getFilepath();
	if (startswith(fpath, "/dev/")) return fpath;
	else return basename_from_path(fpath);
}

void DivIDE::setDiskWritable(bool f) volatile
{
	assert(isMainThread());

	if (cf_card != nullptr) cf_card->setWritable(f);
}

void DivIDE::setRamSize(uint sz)
{
	assert(sz == 32 kB || sz == 512 kB);

	if (sz == ram.count()) return;

	bool f = machine->suspend();
	ram.grow(sz);
	ram.shrink(sz);
	mapMemory();
	if (f) machine->resume();
}
