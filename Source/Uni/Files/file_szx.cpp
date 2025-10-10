// Copyright (c) 2013 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


// https://www.spectaculator.com/docs/svn/zx-state/intro.shtml


#include "file_szx.h"
#include "Items/Ay/AySubclasses.h"
#include "Items/Ay/FullerBox.h"
#include "Items/Fdc/FdcPlus3.h"
#include "Joy/CursorJoy.h"
#include "Joy/KempstonJoy.h"
#include "Joy/SinclairJoy.h"
#include "Joy/Tc2068Joy.h"
#include "Joy/ZxIf2.h"
#include "Machine.h"
#include "Ula/Mmu.h"
#include <zlib.h>

#if defined(__LITTLE_ENDIAN__)
  #define ID(A, B, C, D) ((A << 0) + (B << 8) + (C << 16) + (D << 24))
#elif defined(__BIG_ENDIAN__)
  #define ID(A, B, C, D) ((A << 24) + (B << 16) + (C << 8) + (D << 0))
#else
  #error "endian macro"
#endif


enum ZXST_BlockType : uint32 {
	ZXST_Z80Regs   = ID('Z', '8', '0', 'R'),
	ZXST_SpecRegs  = ID('S', 'P', 'C', 'R'),
	ZXST_AtaSp	   = ID('Z', 'X', 'A', 'T'),
	ZXST_AtaSpRam  = ID('A', 'T', 'R', 'P'),
	ZXST_AY		   = ID('A', 'Y', 0, 0),
	ZXST_CF		   = ID('Z', 'X', 'C', 'F'),
	ZXST_CFRam	   = ID('C', 'F', 'R', 'P'),
	ZXST_Covox	   = ID('C', 'O', 'V', 'X'),
	ZXST_Beta128   = ID('B', '1', '2', '8'),
	ZXST_BetaDisk  = ID('B', 'D', 'S', 'K'),
	ZXST_Creator   = ID('C', 'R', 'T', 'R'),
	ZXST_Dock	   = ID('D', 'O', 'C', 'K'),
	ZXST_GS		   = ID('G', 'S', 0, 0),
	ZXST_GSRamPage = ID('G', 'S', 'R', 'P'),
	ZXST_Keyboard  = ID('K', 'E', 'Y', 'B'),
	ZXST_If1	   = ID('I', 'F', '1', 0),
	ZXST_If2Rom	   = ID('I', 'F', '2', 'R'),
	ZXST_Joystick  = ID('J', 'O', 'Y', 0),
	ZXST_MdrCart   = ID('M', 'D', 'R', 'V'),
	ZXST_Mouse	   = ID('A', 'M', 'X', 'M'),
	ZXST_Multiface = ID('M', 'F', 'C', 'E'),
	ZXST_Opus	   = ID('O', 'P', 'U', 'S'),
	ZXST_OpusDisk  = ID('O', 'D', 'S', 'K'),
	ZXST_Plus3	   = ID('+', '3', 0, 0),
	ZXST_Plus3Disk = ID('D', 'S', 'K', 0),
	ZXST_PlusD	   = ID('P', 'L', 'S', 'D'),
	ZXST_PlusDDisk = ID('P', 'D', 'S', 'K'),
	ZXST_RamPage   = ID('R', 'A', 'M', 'P'),
	ZXST_Rom	   = ID('R', 'O', 'M', 0),
	ZXST_Timex	   = ID('S', 'C', 'L', 'D'),
	ZXST_SIDE	   = ID('S', 'I', 'D', 'E'),
	ZXST_SpecDrum  = ID('D', 'R', 'U', 'M'),
	ZXST_Tape	   = ID('T', 'A', 'P', 'E'),
	ZXST_USpeech   = ID('U', 'S', 'P', 'E'),
	ZXST_ZxPrinter = ID('Z', 'X', 'P', 'R'),
	ZXST_Spectra   = ID('S', 'P', 'R', 'A')
};

struct SzxHeader
{
	/* zx-state header
   The zx-state header appears right at the start of a zx-state (.szx) file.
   It is used to identify the version of the file and to specify which model of ZX Spectrum (or clone) the file
   refers to. Following the zx-state header is an optional ZXSTCREATOR creator information block. Following that
   are the ZXSTZ80REGS and ZXSTSPECREGS blocks followed by zero or more additional blocks representing the current
   state of the emulated Spectrum. */

	static constexpr uint8 AlternateTiming = 1;
	// The emulated Spectrum uses alternate timings (one cycle later than normal timings).
	// If reset, the emulated Spectrum uses standard timings. This flag is only applicable
	// for the ZXSTMID_16K, ZXSTMID_48K and ZXSTMID_128K models. machine_id:

	enum {				   // flags:
		ZX16K		 = 0,  // 16k ZX Spectrum
		ZX48K		 = 1,  // 48k ZX Spectrum or ZX Spectrum+
		ZX128K		 = 2,  // ZX Spectrum 128
		PLUS2		 = 3,  // ZX Spectrum +2
		PLUS2A		 = 4,  // ZX Spectrum +2A/+2B
		PLUS3		 = 5,  // ZX Spectrum +3
		PLUS3E		 = 6,  // ZX Spectrum +3e
		PENTAGON128	 = 7,  // Pentagon 128
		TC2048		 = 8,  // Timex Sinclair TC2048
		TC2068		 = 9,  // Timex Sinclair TC2068
		SCORPION	 = 10, // Scorpion ZS-256
		SE			 = 11, // ZX Spectrum SE
		TS2068		 = 12, // Timex Sinclair TS2068
		PENTAGON512	 = 13, // Pentagon 512
		PENTAGON1024 = 14, // Pentagon 1024
		NTSC48K		 = 15, // 48k ZX Spectrum (NTSC)
		ZX128KE		 = 16  // ZX Spectrum 128Ke
	};

	static constexpr uint32 ZXST_MAGIC = ID('Z', 'X', 'S', 'T');

	uint32 magic;		  // Byte sequence of 'Z', 'X', 'S', 'T' to identify the file as a zx-state file.
	uint8  major_version; // Major version number of the file format. Currently 1.
	uint8  minor_version; // Minor version number of the file format. Currently 4.
	uint8  machine_id;	  // The model of ZX Spectrum (or clone) to switch to when loading the file.
	uint8  flags;		  // ALTERNATETIMINGS

	Model getZxspModel();
};

static_assert(sizeof(SzxHeader) == 8, "will be read/written en bloc");

Model SzxHeader::getZxspModel()
{
	if (magic != ZXST_MAGIC) return unknown_model;

	if (flags & AlternateTiming) logline(".szx: AlternateTiming set (TODO)");

	switch (machine_id)
	{
	case ZX16K: return zxsp_i1; // alt. timing ignored
	case ZX48K: return zxsp_i3; // alt. timing ignored
	case ZX128K: return zx128;
	case PLUS2: return zxplus2;
	case PLUS2A: return zxplus2a;
	case PLUS3: return zxplus3;
	case PLUS3E: break;
	case PENTAGON128: return pentagon128;
	case TC2048: return tc2048;
	case TC2068: return tc2068;
	case SCORPION: break; // return scorpion;
	case SE: break;
	case TS2068: return ts2068;
	case PENTAGON512: break;
	case PENTAGON1024: break;
	case NTSC48K: return tk95;
	case ZX128KE: break;
	}
	return unknown_model; // model not supported
}

enum ZXSTZF { EILAST = 1, HALTED = 2, FSET = 4 };
static constexpr uint Z80RegsSize		= 37;
static constexpr uint ZXSTRF_COMPRESSED = 1;
static constexpr uint ZXSTKF_ISSUE2		= 1;


// ======================================================================
// Implementations


Model modelForSZX(FD& fd)
{
	/*  determine required model for loading this .z80 snapshot.
		Return unknown_model = Error or not supported.
	*/
	SzxHeader head;
	off_t	  p = fd.file_position();
	fd.read(head);
	Model model = head.getZxspModel();

	// for 48k model check whether it is issue 2 or 3:
	// zx48k: default is issue 3, if no ZXST_Keyboard present in the file.
	// zx16k: we always use issue 2 ear-in behavior, despite szx specs.
	if (model == zxsp_i3)
	{
		while (!fd.is_at_eof())
		{
			uint32 id	= fd.read_uint32_z();
			uint32 size = fd.read_uint32_z();

			if (id != ZXST_Keyboard)
			{
				fd.skip_bytes(size);
				continue;
			}
			else
			{
				uint32 flags = fd.read_uint32_z();
				if (flags & ZXSTKF_ISSUE2) model = zxsp_i2;
				break;
			}
		}
	}

	fd.seek_fpos(p);
	return model;
}

void Machine::szx_add_joystick(uint if_id, JoystickID js_id)
{
	// attach joystick.
	// possibly open joystick toolwindow.
	// TODO: remove all wrong external joysticks
	// TODO: insert joystick

	enum {
		KEMPSTON  = 0, // Kempston joystick emulation
		FULLER	  = 1, // Fuller joystick emulation
		CURSOR	  = 2, // Cursor (AGF or Protek) emulation
		SINCLAIR1 = 3, // Sinclair Interface II port 1 (or Spectrum +2A/+3 joystick 1)
		SINCLAIR2 = 4, // Sinclair Interface II port 2 (or Spectrum +2A/+3 joystick 2)
		ZXPLUS	  = 5, // Spectrum+/128/+2/+2A/+3 cursor keys (if Keyboard JS setting)
		COMCOM	  = 5, // Comcom programmable joystick interface (if Joystick setting)
		TIMEX1	  = 6, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 1.
		TIMEX2	  = 7, // Timex TC2048, TC2068, TS2068 and Spectrum SE built-in joystick, port 2.
		NONE	  = 8  // None
	};

	static constexpr cstr jsname[] = {"no joystick", "keyboard emulation", "USB joy#1", "USB joy#2", "USB joy#3"};
	static constexpr cstr ifname[] = {"Kempston",		  "Fuller", "Cursor", "Sinclair1", "Sinclair2",
									  "zxplus_or_comcom", "Timex1", "Timex2", "none"};

	assert(js_id <= usb_joystick2);
	if (if_id >= NONE) return;
	logline(".szx: use %s joystick interface for %s", ifname[if_id], jsname[js_id]);

	Item* joy  = nullptr;
	uint  port = 0;

	switch (if_id)
	{
	case KEMPSTON: //
		joy = addExternalItem(isa_KempstonJoy);
		break;

	case FULLER:
		joy = addExternalItem(isa_FullerBox); // currently emulates only the AY chip
		showWarning(
			"Fuller joystick emulation requested. This is not yet implemented. "
			"If you need this please file a bug report. (TODO)");
		break;

	case CURSOR:
		if (!(joy = find<CursorJoy>())) joy = addExternalItem(isa_ProtekJoy);
		break;

	case SINCLAIR1:
	case SINCLAIR2:
		port = if_id - SINCLAIR1;
		if (!(joy = find<SinclairJoy>())) joy = addExternalItem(isa_ZxIf2);
		break;

	case ZXPLUS:				   // or COMCOM:
		if (js_id == kbd_joystick) // ZXPLUS
			// cursor joystick but for the plus, 128, +2, +2A, +3 (probably with symshift)
			// this remaps keys to other keys.
			// this is not implemented. (no such external or internal interface emulated)
			// not remapping the keys is probably the best solution:
			showWarning(
				"zxplus cursor key remapping requested. This is not implemented. "
				"Please use the cursor keys themselves.");
		else // COMCOM
			showWarning("COMCOM joystick emulation requested. Not implemented.");
		return;

	case TIMEX1:
	case TIMEX2:
		port = if_id - TIMEX1;
		// the machine is expected to have them,
		// otherwise we'd need also the Timex AY:
		if (!(joy = find<Tc2068Joy>())) showWarning("Times joystick requested but this is no Times machine: ignored");
		break;

	default: // dummy
		return;
	}

	// insert a joystick as requested:
	// this is probably not a good idea:
	// keys may become dead unexpectedly or the USB joystick does not exist.
	// TODO: open the joystick toolwindow?
	assert(dynamic_cast<Joy*>(joy));
	//static_cast<Joy*>(joy)->insertJoystick(port, js_id);
}

void Machine::loadSZX(FD& fd)
{
	xlogIn("Machine:loadSZX");

	assert(is_locked());
	__unused bool ismainthread = isMainThread();

	SzxHeader head;
	fd.read(head);
	assert(head.magic == SzxHeader::ZXST_MAGIC);
	assert(model == head.getZxspModel());

	while (!fd.is_at_eof())
	{
		uint32 id		  = fd.read_uint32_z(); // Intel byte order
		uint32 size		  = fd.read_uint32_z();
		off_t  fpos_start = fd.file_position();

		logline(".szx: '%.4s' size = %u", cstr(&id), size);

		switch (id)
		{
		case ZXST_Z80Regs:
		{
			assert(cpu);
			if (size != Z80RegsSize) logline(".szx: ZXSTZ80REGS size = %u", size);

			Z80Regs regs;
			regs.af				= fd.read_uint16_z();
			regs.bc				= fd.read_uint16_z();
			regs.de				= fd.read_uint16_z();
			regs.hl				= fd.read_uint16_z();
			regs.af2			= fd.read_uint16_z();
			regs.bc2			= fd.read_uint16_z();
			regs.de2			= fd.read_uint16_z();
			regs.hl2			= fd.read_uint16_z();
			regs.ix				= fd.read_uint16_z();
			regs.iy				= fd.read_uint16_z();
			regs.sp				= fd.read_uint16_z();
			regs.pc				= fd.read_uint16_z();
			regs.i				= fd.read_uint8();
			regs.r				= fd.read_uint8();
			regs.iff1			= fd.read_uint8() ? enabled : disabled;
			regs.iff2			= fd.read_uint8() ? enabled : disabled;
			regs.im				= fd.read_uint8() % 3;
			int32 cc			= fd.read_int32_z();
			uint8 irq_remaining = fd.read_uint8();
			uint8 flags			= fd.read_uint8();
			// uint16 mem_ptr	= fd.read_uint16_z();
			assert(fd.file_position() == fpos_start + Z80RegsSize - 2);

			_suspend();
			_power_on(cc);

			assert(current_cc() == cc);
			assert(now() == 0.0);

			cpu->getRegisters() = regs;

			if (irq_remaining)
			{
				// normal zxsp irq cc = 0..48, Pentagon 0..36
				logline(".szx: cc + irq_remaining = %u + %u = %u", cc, irq_remaining, cc + irq_remaining);
				cpu->setInterrupt(0, cc + irq_remaining);
			}
			if (flags & ZXSTZF::EILAST) logline(".szx: EILAST set: TODO"); //TODO
			if (flags & ZXSTZF::HALTED) logline(".szx: HALTED set: TODO"); //TODO
			if (flags & ZXSTZF::FSET) logline(".szx: flag 'FSET' set: ignored");
			break;
		}
		case ZXST_Creator:
		{
			if (size < 32 + 2 + 2) logline(".szx: ZXSTCREATOR size = %u", size);
			char creator[32];
			fd.read_bytes(creator, 32);
			uint16 major = fd.read_uint16_z();
			uint16 minor = fd.read_uint16_z();
			for (uint i = 0; i < 32 && creator[i]; i++)
				if (creator[i] < 32 || creator[i] > 126) creator[i] = '?';
			logline(".szx: creator = %.31s %u.%u", creator, major, minor);
			break;
		}
		case ZXST_SpecRegs:
		{
			assert(ula);
			assert(mmu);
			if (size != 8) logline(".szx: ZXSTSPECREGS size = %u", size);

			uint8 border = fd.read_uint8();
			uint8 ch7ffd = fd.read_uint8();
			uint8 ch1ffd = fd.read_uint8(); // also port 0xeff7 of Pentagon1024
			uint8 chfffe = fd.read_uint8();

			ula->set_ula_out_byte(chfffe); // low level: just the data member
			ula->setBorderColor(border);   // may be different than chfffe
			mmu->setPort7ffd(ch7ffd);
			mmu->setPort1ffd(ch1ffd);
			break;
		}
		case ZXST_RamPage:
		{
			uint16 flags   = fd.read_uint16_z();
			uint8  page_no = fd.read_uint8();
			uchar  ucbu[16 kB];

			if (flags & ZXSTRF_COMPRESSED)
			{
				logline(".szx page_no = %u (compressed)", page_no);

				uint32					 csize	= size - 3;
				ulong					 ucsize = 16 kB;
				std::unique_ptr<uchar[]> cbu {new uchar[csize]};
				fd.read_bytes(cbu.get(), csize);
				int err = uncompress(&ucbu[0], &ucsize, &cbu[0], csize);
				if (err != Z_OK) throw usingstr(".szx ram page %u: zlib error %u", page_no, err);
				if (ucsize != 16 kB) throw usingstr(".szx ram page %u: wrong size: %lu", page_no, ucsize);
			}
			else
			{
				logline(".szx page_no = %u (uncompressed)", page_no);

				if (size != 16 kB + 3) logline(".szx: ZXSTSPECREGS wrong uncompressed block size");
				fd.read_bytes(ucbu, 16 kB);
			}

			bool paged_mem = mmu->hasPort7ffd();
			if (!paged_mem)
			{
				// page 5: 0x4000 - 0x7fff
				// page 2: 0x8000 - 0xbfff
				// page 0: 0xc000 - 0xffff
				if (page_no != 5 && page_no != 2 && page_no != 0) throw usingstr(".szx: illegal page no: %u", page_no);
				page_no = page_no == 5 ? 0 : page_no == 2 ? 1 : 2;
			}

			if (page_no * 16 kB >= ram.count()) // built-in ram
				throw usingstr(".szx: page no exceeds ram: %u", page_no);

			Z80::b2c(ucbu, &ram[page_no * 16 kB], 16 kB);
			break;
		}
		case ZXST_ZxPrinter:
		{
			if (size != 2) logline(".szx: ZXSTZXPRINTER wrong size = %u", size);

			static constexpr uint16 ENABLED = 1;
			uint16					flags	= fd.read_uint16_z();
			if (flags & ENABLED) logline(".stx: ZX Printer enabled (not supported)");
			break;
		}
		case ZXST_Keyboard:
		{
			// keyboard joystick emulation
			// and issue2 emulation flag

			if (size != 4 && size != 5) logline(".szx: ZXSTKEYBOARD wrong size = %u", size);

			// Test if Issue 2 keyboard emulation is enabled.
			// This is only applicable for the 16k or 48k ZX Spectrum.
			uint32 flags = fd.read_uint32_z();
			if (flags & ZXSTKF_ISSUE2)
			{
				if (model != zxsp_i1 && model != zxsp_i2)
					logline(
						".stx: file indicates issue 2 ear-in emulation but model used is a %s", model_info->nickname);
			}
			else
			{
				if (model == zxsp_i1 || model == zxsp_i2)
					logline(
						".stx: model used is a %s but file does not indicate issue 2 ear-in emulation",
						model_info->nickname);
			}

			// attach joystick.
			if (size >= 5)
			{
				uint joystick_type = fd.read_uint8();
				szx_add_joystick(joystick_type, kbd_joystick);
			}
			break;
		}
		case ZXST_Joystick:
		{
			// Joystick setup for both players.

			if (size != 6) logline(".szx: ZXSTJOYSTICK wrong size = %u", size);

			constexpr uint32 ALWAYSPORT31 = 1;

			uint32 flags = fd.read_uint32_z();
			if (flags & ALWAYSPORT31) logline(".stx: always port 31 set (deprecated, ignored, todo?)"); //TODO

			uint joystick1_type = fd.read_uint8();
			uint joystick2_type = fd.read_uint8();
			szx_add_joystick(joystick1_type, num_usb_joysticks >= 1 ? usb_joystick0 : kbd_joystick);
			szx_add_joystick(joystick2_type, num_usb_joysticks >= 2 ? usb_joystick1 : kbd_joystick);
			break;
		}
		case ZXST_AY:
		{
			// ZXSTAYBLOCK
			// The state of the AY chip found in all 128k Spectrums, Pentagons, Scorpions and Timex machines.
			// This block may
			// also be present for 16k/48k Spectrums if Fuller Box or Melodik emulation is enabled. AY Block.
			// Contains the
			// AY register values

			if (size != 18) logline(".szx: ZXSTAYBLOCK wrong size = %u", size);

			enum {
				BUILT_IN  = 0, // 128k
				FULLERBOX = 1, // Fuller Box emulation.
				ZX128AY	  = 2  // Melodik Soundbox emulation. (external 128K compatible sound box)
			};

			uint8 flags		   = fd.read_uint8();
			uint8 selected_reg = fd.read_uint8();
			uint8 regs[16];
			fd.read_bytes(regs, 16);

			// attach AY sound chip:
			if (flags == FULLERBOX)
			{
				if (ay && ay->isExternal() && !dynamic_cast<FullerBox*>(ay)) removeItem(ay);
				addExternalItem(isa_FullerBox);
			}
			else
			{
				removeItem(isa_FullerBox);
				if (!ay) addExternalItem(isa_DidaktikMelodik);
			}

			// note: in rare cases, if 2 AY devices were present to begin with, this may be the wrong one:
			assert(ay);
			ay->setRegisters(regs);
			ay->setRegNr(selected_reg);
			break;
		}
		case ZXST_Plus3:
		{
			// ZXSTPLUS3
			// +3 disk drives
			// The number of drives connected to the Spectrum +3 and whether their motors are turned on.
			// Any ZXSTDSKFILE blocks specifying which disk files are in which drive will follow this one.

			if (size != 2) logline(".szx: ZXSTPLUS3 wrong size = %u", size);

			uint8 num_drives = fd.read_uint8(); // 1 or 2
			uint8 motor_on	 = fd.read_uint8(); // 0 or 1

			logline(".szx: num drives = %u %s", num_drives, motor_on ? " (running)" : "");
			if (num_drives > 1) logline(".szx: 2nd drive not supported");

			FdcPlus3* drive = find<FdcPlus3>();
			if (!drive) logline(".stx: no plus3 disk controller found");
			if (!drive) break;
			drive->setMotor(now(), motor_on);
			break;
		}
		case ZXST_Plus3Disk:
		case ZXST_Multiface:
		case ZXST_AtaSp:
		case ZXST_AtaSpRam:
		case ZXST_CF:
		case ZXST_CFRam:
		case ZXST_Covox:
		case ZXST_Beta128:
		case ZXST_BetaDisk:
		case ZXST_Dock:
		case ZXST_GS:
		case ZXST_GSRamPage:
		case ZXST_If1:
		case ZXST_If2Rom:
		case ZXST_MdrCart:
		case ZXST_Mouse:
		case ZXST_Opus:
		case ZXST_OpusDisk:
		case ZXST_PlusD:
		case ZXST_PlusDDisk:
		case ZXST_Rom:
		case ZXST_Timex:
		case ZXST_SIDE:
		case ZXST_SpecDrum:
		case ZXST_Tape:
		case ZXST_USpeech:
		case ZXST_Spectra:
		default: logline(".szx: '%.4s' not yet supported", cstr(&id)); break;
		}

		fd.seek_fpos(fpos_start + size);
	}

	xlogline("loaded ok");
}

void Machine::saveSZX(FD&) {}

/*


































*/
