// Copyright (c) 2013 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TccRom.h"
#include "Machine.h"
#include "RecentFilesMenu.h"
#include "Z80/Z80.h"
#include "unix/FD.h"
#include "unix/files.h"


/*	DCK files keeps information about memory content of various Timex memory expansions,
	and information which chunks of extra memory are RAM chunks and which chunks are ROM chunks.
	Such files have relatively simple format. At the beginning of a DCK file, a nine-byte header is located.
	First byte is bank ID with following meaning:

	0:		DOCK bank:	the most frequent variant
	1-253: 	Reserved
	254: 	EXROM bank:	RAM or ROM in EXROM bank, such hardware units exist on real Timex Sinclair
	255: 	HOME bank:	mainly useless, HOME content is typically stored in a Z80 file
						however, using this bank ID you may replace content of Timex HOME ROM or turn Timex HOME ROM
   into RAM

	This numbering of banks is in according to convention used in various routines from Timex ROM.
	After the first byte, following eight bytes corresponds to eight 8K chunks in the bank. Organization of each byte is
   as follows:

	bit D0:			1 = writable chunk
	bit D1:			1 = memory image is present in DCK file
	bits D2-D7:		reserved (set to zero)

	=>	0 = non-existent chunks: reading from such chunks must return default values for given bank;
			for example, #FF in DOCK bank, and ghost images of 8K Timex EXROM in EXROM bank
		1 = RAM chunks, where initial RAM content is not given
		2 = ROM chunk
		3 = RAM chunks with initial memory contents: for saving of expanded RAM or emulating non-volatile RAM expansions

	After the header, a pure image of each presented chunk is stored in DCK file.

	That's all if only one bank is stored in the DCK file. Else a new 9-byte header for the next bank follows, and so
   on.
*/

#define DOCK  0
#define EXROM 254
#define HOME  255


// This must match TccRom.Id:
uint32 hashes[] = {
	0x00000000, // EmptyTcc or pure Ram TCCs
				// these are the known official ones:
	0x711BC8B4,
	0x291D4645, // Androids,Budgeter
	0x7AF2664A,
	0x64DACE8C, // Casino1,CrazyBugs,
	0xCCE6105F,
	0xFCF48C07, // FlightSimulator,Pinball,
	0x6CE0D15B,
	0xA764667E, // StatesAndCapitals,Penetrator,
				// non-releases:
	0xC75FD9A9,
	0x70D353F3, // Backgammon,Chess
	0x524C549B, // Gyruss,
	0x7EBC77C9,
	0xF8A68738, // HoraceAndTheSpiders,HungryHorace,
	0x2BC430ED,
	0x689A1AA6, // LocoMotion,MontezumasRevenge,
	0xF8CF0C1C,
	0x63CD4488, // Planetoids,Popeye
	0xAC486933,
	0xEA9A6A7F, // QBert,ReturnOfTheJediDeathStarBattle,
	0xA00FBEDB,
	0x2014FE7C, // ShadowOfTheUnicorn,SpaceRaiders,
	0x4F2AD26C,
	0x851827A0, // StarWarsTheArcadeGame,Swordfight,
				// utilities etc.:
	0xE00804AD,
	0xC34D5E38, // EToolkit,MTerm
	0xAE8E3E3D, // Rwp32,
	0x58995A0B, // SuperHotZDisassemblerV251: AROS and EXROM versions have identical roms
	0x2109D635,
	0x61A8EA54, // TaswordII,TimeWord
	0x82FA73FF,
	0xF65BC8D5, // VuCalc,VuFile,
	0xEE4CAFA5,
	0x4163E796, // ZebraOS64,Tc2048Emu
	0xAEDD9AE3, // JupiterAceEmu,
	0x4C1FDCC8,
	0x845E7691 // ZxseEmu,ZxspEmu
};


// helper
void read_buffer(FD& fd, CoreByte zbu[0x2000], uint64& hash) throws
{
	union
	{
		uint8  bu8[0x2000];
		uint64 bu64[0x0400];
	};
	fd.read_bytes(bu8, 0x2000);
	for (int i = 0; i < 0x400; i++) hash += bu64[i];
	Z80::b2c(bu8, zbu, 0x2000);
}


/*	Lade ein Timex Dock Cartridge vom File:
	EXROM und DOCK banks werden in die entsprechenden Arrays geladen
	HOME banks werden ins interne Rom und ggf. Ram geladen, das zuvor nach rom[] gerettet wird.
*/
TccRom::TccRom(Machine* machine, cstr path) :
	machine(machine), fpath(newcopy(path)),
	id(TccUnknown), exrom {0, 0, 0, 0, 0, 0, 0, 0}, dock {0, 0, 0, 0, 0, 0, 0, 0}, rom(nullptr), exrom_r(0), exrom_w(0),
	exrom_d(0), dock_r(0), dock_w(0), dock_d(0), home_r(0), home_w(0), home_d(0)
{
	xlogline("new TccRom(\"%s\")", path);

	try
	{
		uint64 hash = 0;
		FD	   fd(path, 'r');

		while (fd.file_remaining() >= 9)
		{
			uint bank = fd.read_uint8();
			if (bank && bank < 254) throw DataError("Invalid bank Id: 0x%02X", int(bank));
			xlogline("  Bank = 0x%02X", uint(bank));

			uint8 flags[8];
			fd.read_bytes(flags, 8);
			for (int i = 0; i < 8; i++)
				if (flags[i] > 3) throw DataError("Invalid block flag: %02X", int(flags[i]));
#ifdef XXLOG
			log("chunks: ");
			for (int i = 0; i < 8; i++) log("%i ", int(flags[i]));
			logNl();
#endif

			uint r = 0, w = 0, d = 0; // readable, writable, data provided
			for (int i = 8; i--;)
			{
				int f = flags[i];
				r += r + (f != 0);	  // block readable? (present?)
				w += w + (f & 1);	  // block writable?
				d += d + (f & 2) / 2; // block has init data?
			}

			if (bank == DOCK) // data is loaded into dock[]
			{
				dock_r = r;
				dock_w = w;
				dock_d = d;

				for (int i = 0; i < 8; i++)
				{
					if (d & (1 << i)) xxlogline("  reading block");
					//					if(r&(1<<i)) dock[i].grow(0x2000);						// bank present => allocate
					//ram
					if (r & (1 << i))
						dock[i] = new Memory(
							machine, usingstr("TCC Dock Bank %i", i), 0x2000);	// bank present => allocate ram
					if (d & (1 << i)) read_buffer(fd, dock[i].getData(), hash); // init data present => read it
				}
			}

			else if (bank == EXROM) // data is loaded into exrom[]
			{
				exrom_r = r;
				exrom_w = w;
				exrom_d = d;

				for (int i = 0; i < 8; i++)
				{
					if (d & (1 << i)) xxlogline("  reading block");
					//					if(r&(1<<i)) exrom[i].grow(0x2000);						// bank present => allocate
					//ram
					if (r & (1 << i))
						exrom[i] = new Memory(
							machine, usingstr("TCC Exrom bank %i", i), 0x2000);	 // bank present => allocate ram
					if (d & (1 << i)) read_buffer(fd, exrom[i].getData(), hash); // init data present => read it
				}
			}

			else // the HOME bank is loaded into the internal ram&rom of the machine
			{	 // if the internal rom is overwritten, it is saved into this.rom[]
				home_w = w;
				home_r = r;
				home_d = d;

				if ((r & 3) && !rom) // save internal rom data
				{
					rom = new uint8[0x4000];
					Z80::c2b(machine->rom.getData(), rom, 0x4000);
				}

				for (int i = 0; i < 8; i++)
				{
					if (d & (1 << i)) // read init data
					{
						xlogline("  reading block");
						if (i < 2)
							read_buffer(fd, &machine->rom[0x2000 * i], hash);
						else
							read_buffer(fd, &machine->ram[0x2000 * (i - 2)], hash);
					}
				}
			}
		}
		uint32 hash32 = hash + (hash >> 32);
		for (TccRomId i = (TccRomId)0; i < NELEM(hashes); i = (TccRomId)(i + 1))
		{
			if (hash32 == hashes[i])
			{
				id = i;
				break;
			}
		}
		if (hash32 && id == TccUnknown) logline("Tcc Rom Cartridge \"%s\" with unknown hash: %08X", path, hash32);

		addRecentFile(RecentTccRoms, path);
		addRecentFile(RecentFiles, path);
	}
	catch (AnyError& e)
	{
		showAlert("An error occured while reading file \"%s\":\n%s", filename_from_path(fpath), e.what());
		dock_d = exrom_d = home_d = 0x00; // prevent write-on-close
	}
}


/*	Destructor:
	If the rom cartridge contained Ram with init data (NVRam) then the .dck file is updated (written back to).
	If the cartridge contained HOME bank pages, then the machine's Rom is restored from this.rom[].
	The memory is not unmapped by this destructor. This must be done by beforehand by the caller.
*/
TccRom::~TccRom()
{
	if ((dock_d & dock_w) | (exrom_d & exrom_w) | (home_d & home_w)) // write back NVRam contents
	{
		try
		{
			save_as(fpath);
		}
		catch (FileError& e)
		{
			showAlert("Writing back the NVRam of the cartridge failed:\n%s", e.what());
		}
	}

	if (rom) // restore internal rom
	{
		Z80::b2c(rom, machine->rom.getData(), 0x4000);
		delete[] rom;
	}

	delete[] fpath;
}


void TccRom::saveAs(cstr filepath)
{
	try
	{
		save_as(filepath);
		delete[] fpath;
		fpath = newcopy(filepath);
		addRecentFile(RecentTccRoms, filepath);
		addRecentFile(RecentFiles, filepath);
	}
	catch (FileError e)
	{
		showAlert("Writing to file \"%s\" failed:\n%s", filename_from_path(filepath), e.what());
	}
}


void TccRom::save_as(cstr fpath) throws // AnyError
{
	FD fd;
	fd.open_file_w(fpath);
	uint8 bu[0x2000];

	if (home_r) // save HOME bank data
	{
		fd.write_uint8(HOME); // bank id
		uint r = home_r, w = home_w, d = home_d;
		for (int8 m = 1; m; m += m) fd.write_uint8(r & m ? (w & m) + (d & m) * 2 : 0x00); // 8 chunk flags

		for (int i = 0; i < 8; i++)
		{
			if (d & (1 << i))
			{
				if (i < 2)
					Z80::c2b(&machine->rom[0x2000 * i], bu, 0x2000);
				else
					Z80::c2b(&machine->ram[0x2000 * (i - 2)], bu, 0x2000);
				fd.write_bytes(bu, 0x2000);
			}
		}
	}

	if (dock_r)
	{
		fd.write_uint8(DOCK); // bank id
		uint r = dock_r, w = dock_w, d = dock_d;
		for (int8 m = 1; m; m += m) fd.write_uint8(r & m ? (w & m) + (d & m) * 2 : 0x00); // 8 chunk flags
		for (int i = 0; i < 8; i++)
		{
			if (d & (1 << i))
			{
				Z80::c2b(dock[i].getData(), bu, 0x2000);
				fd.write_bytes(bu, 0x2000);
			}
		}
	}

	if (exrom_r)
	{
		fd.write_uint8(EXROM); // bank id
		uint r = exrom_r, w = exrom_w, d = exrom_d;
		for (int8 m = 1; m; m += m) fd.write_uint8(r & m ? (w & m) + (d & m) * 2 : 0x00); // 8 chunk flags
		for (int i = 0; i < 8; i++)
		{
			if (d & (1 << i))
			{
				Z80::c2b(exrom[i].getData(), bu, 0x2000);
				fd.write_bytes(bu, 0x2000);
			}
		}
	}
}
