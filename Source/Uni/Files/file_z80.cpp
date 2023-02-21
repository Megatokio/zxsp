// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Ay/Ay.h"
#include "Ay/AySubclasses.h"
#include "Ay/FullerBox.h"
#include "Fdc/Fdc.h"
#include "Items/SpectraVideo.h"
#include "Items/Ula/MmuPlus3.h"
#include "Items/Ula/Ula.h"
#include "Joy/Joy.h"
#include "Machine.h"
#include "MachineController.h"
#include "Ram/Cheetah32kRam.h"
#include "Ram/Memotech64kRam.h"
#include "Ram/Zx16kRam.h"
#include "Ram/Zx3kRam.h"
#include "Screen/Screen.h"
#include "Ula/Mmu128k.h"
#include "Z80/Z80.h"
#include "Z80Head.h"
#include "ZxIf1.h"
#include "unix/FD.h"
#include <QWidget>


/* ----	write compressed .z80 block -------------------------------------------
		writes block header and compressed data for v2.01 or later
		block layout:
			dc.w		length of data (without this header; low byte first)
			dc.b		page number of block
			dc.s		compressed data follows
		compression scheme:
			dc.b $ed, $ed, count, char
*/
static void write_compressed_page(FD& fd, uint8 flag, const CoreByte* q, uint qsize)
{
	xlogIn("write_compressed_page(%i)", int(flag));

	assert(qsize >= 1 kB && qsize <= 64 kB);

	const CoreByte* qe = q + qsize;
	uint8			bu[qsize * 5 / 3]; // worst case size: 5/3*qsize
	uint8*			z = bu;

	while (q < qe)
	{
		uint8 c = *q++;
		if (q == qe || uint8(*q) != c) // single byte
		{
			*z++ = c;
			// special care for compressible sequence after single 0xed:
			if (c == 0xed && q + 2 <= qe && uint8(*q) == uint8(*(q + 1))) { *z++ = *q++; }
		}
		else // sequence of same bytes
		{
			int n = 1;
			while (n < 255 && q < qe && uint8(*q) == c)
			{
				n++;
				q++;
			}
			if (n >= 4 || c == 0xed)
			{
				*z++ = 0xed;
				*z++ = 0xed;
				*z++ = n;
				*z++ = c;
			} // compress ?
			else
			{
				while (n--) *z++ = c;
			} // don't compress
		}
	}

	uint zsize = z - bu;
	fd.write_uint16_z(zsize);
	fd.write_char(flag);
	fd.write_bytes(bu, zsize);
}


/*  read an uncompressed v1.45 block
 */
static void read_uncompressed_page(FD& fd, CoreByte* z, uint size)
{
	uint8 bu[size];
	fd.read_bytes(bu, size);
	Z80::b2c(bu, z, size);
}


/* ----	read compressed .z80 block -----------------------------------------
		read data from a v1.45 or v2.01 or later compressed data block
		does not read the block header, only the compressed data:
			wg. v1.45: has no header and
				v2.01: caller needs to know the flag byte for destination
		in: fd      input file
			qsize   source size from block header
					0xFFFF: uncompressed 0x4000 bytes
			z       destination pointer: -> uint16[] Memory: flags in upper byte are preserved
			zsize   destination size:    must match decoded data
		throws on error
*/
static void read_compressed_page(FD& fd, uint qsize, CoreByte* z, uint zsize)
{
	if (qsize == 0xFFFF && zsize == 0x4000)
	{
		xlogline("Hooray! an uncompressed page in a .z80 file!");
		read_uncompressed_page(fd, z, zsize);
		return;
	}

	uint8 bu[qsize];
	fd.read_bytes(bu, qsize);

	uint8	  c, n;
	uint8*	  q		= bu;
	uint8*	  q_end = q + qsize;
	CoreByte* z_end = z + zsize;

#define store(c) reinterpret_cast<FourBytes*>(z++)->data = c

	while (z < z_end && q < q_end)
	{
		c = *q++; // read next byte
		if (c != 0xed)
		{
			store(c);
			continue;
		} // single byte
		if (q == q_end)
		{
			store(c);
			continue;
		} // rare case: last source byte is 0xed

		c = *q++; // read next byte
		if (c != 0xed)
		{
			store(0xed);
			q--;
			continue;
		};

		n = *q++;
		c = *q++; // read count & char
		if ((z + n) > z_end) goto e;
		if (q > q_end) goto e;
		while (n--) store(c); // expand compressed bytes
	};

	if (q == q_end && z == z_end)
	{
		//  uint16 zbu[zsize];
		//  z = z_end-zsize;
		//	decompress_z80(bu,qsize,zbu,zsize);
		//  for(uint i=0;i<zsize;i++)
		//  {
		//      if((zbu[i]&0x00FF)!=(z[i]&0x0FF)) IERR();
		//  }
		//	logline("decompressed page validated ok");

		return; // ok
	}

	if (z == z_end && q + 4 <= q_end && zsize == 0xC000 && // version 1.45 file with end marker ?
		*q++ == 0 && *q++ == 0xed && *q++ == 0xed && *q++ == 0)
		return;

e:
	throw DataError("File corrupted: decoding of a compressed block failed");
}


/*  save .z80 file; version 3.00
 */
void Machine::saveZ80(FD& fd)
{
	xlogIn("Machine:saveZ80");

	Z80Head head;
	head.setRegisters(cpu->getRegisters());

	head.h2lenl = z80v3len - 2 - z80v1len;

	head.data |= ula->getBorderColor() << 1;

	Joy* kj = (Joy*)findIsaItem(isa_KempstonJoy);
	Joy* sj = (Joy*)findIsaItem(isa_SinclairJoy);
	// Bit 6-7: 0=Cursor/Protek/AGF joystick
	if (kj) head.im |= 1 << 6;							  // Bit 6-7: 1=Kempston
	else if (sj && sj->isConnected(0)) head.im |= 3 << 6; //          3=IF2 right JS
	else if (sj && sj->isConnected(1)) head.im |= 2 << 6; //          2=IF2 left JS

	ZxIf1* if1 = (ZxIf1*)findItem(isa_ZxIf1);
	if (if1) showWarning("Interface 1: TODO");
	Item* mgt = findItem(isa_MGT);
	if (mgt) showWarning("M.G.T. interface: TODO"); // probably never
	Model model = this->model == zxsp_i1 && ram.count() > 0x4000 ? zxsp_i2 : this->model;
	head.setZxspModel(model, if1, mgt);
	if (ula->is60Hz() && (ula->isA(isa_UlaZx80) || ula->isA(isa_UlaJupiter))) head.im |= 0x04;

	bool varying_ramsize = head.varyingRamsize();
	if (varying_ramsize) head.spectator = ram.count() / 0x400;

	// port_7ffd:	// In SamRam mode: bitwise state of 74ls259.
	// In 128 mode:    last OUT to 7ffd (paging control)
	// Timex TS2068:   last out to port F4
	head.port_7ffd = mmu->hasPort7ffd() ? mmu->getPort7ffd() : mmu->hasPortF4() ? mmu->getPortF4() : 0;

	// if1paged:	// !=0 means: interface1 rom is paged in
	// Timex TS2068: last out to port FF
	head.if1paged = if1 ? if1->isRomPagedIn() : ula->hasPortFF() ? ula->getPortFF() : 0;

	// port_fffd: (byte 38)			// Last OUT to fffd (soundchip register number)
	// soundreg[16]: (byte 39-54)	// Contents of the sound chip registers
	// rldiremu: (byte 37)			// Bit 0: 1 if R register emulation on
	// Bit 1: 1 if LDIR emulation on
	// Bit 2: AY sound in use, even on 48K machines
	// Bit 6: (if bit 2 set) Fuller Audio Box emulation
	// Bit 7: Modify hardware: 48k->16k, 128->+2, +3->+2A

	head.rldiremu |= 0x03; // R register and LDIR emu: always on

	Ay* ay = this->ay;
	if (!ay) ay = AyPtr(findIsaItem(isa_Ay));
	if (ay)
	{
		head.rldiremu |= (1 << 2);								 // ay in use, even on 48k machine
		if (find(ay->name, "Fuller")) head.rldiremu |= (1 << 6); // AY chip for Fuller box
		head.port_fffd = ay->getRegNr();						 // selected register
		memcpy(head.soundreg, ay->getRegisters(), 16);			 // registers
	}

	SpectraVideo* spectra = findSpectraVideo();
	if (spectra)
	{
		head.h2lenl = max(uint(head.h2lenl), z80v3len - 2u - z80v1len + 3u);
		head.rldiremu |= 0x08;
		head.spectra_port_7fdf = spectra->getVideoMode();
		head.spectra_bits	   = (spectra->newVideoModesEnabled() << 0) + (spectra->rs232_enabled << 1) +
							(spectra->joystick_enabled << 2) + (spectra->if1_rom_hooks_enabled << 3) +
							(spectra->isRomPagedIn() << 4) + ((spectra->port_239 & 1) << 5) +
							((spectra->port_239 & 0x10) << 2) + (spectra->port_247 << 7);
		head.data |= spectra->getBorderColor() & 0xE0;
	}

	// T state counter: (byte 55-57)
	head.setCpuCycle(cpu->cpuCycle(), ula->cpuCycleOfFrameFlyback());
	assert(head.getCpuCycle(ula->cpuCycleOfFrameFlyback()) == cpu->cpuCycle());

	// Flag used by Spectator:
	// head.spectator = 0;

	// MGT rom paged in?            // 0xFF if MGT Rom paged
	// head.mgt_paged = 0;

	// Multiface rom paged in?      // 0xFF if Multiface Rom paged. Should always be 0.
	// head.multiface_paged = 0;

	// Soft ROM?                    // 0xFF if 0-8191 / 8192-16383 is ROM
	head.ram0 = head.ram1 = 0xff; // probably only valid if Multiface or similar attached

	// Joystick keyboard emulation:			this is PC HW dependent!
	// head.joy[10],                // 5* ascii word: keys for user defined joystick
	// head.stick[10],              // 5* keyboard mappings corresponding to keys above

	// MGT type:                    // MGT type: 0=Disciple+Epson,1=Disciple+HP,16=Plus D
	// head.mgt_type = 0;

	// Disciple inhibit button status:		0=out, 0xFF=in
	// disciple_inhibit_button_status = 0;

	// Disciple inhibit flag:				0=rom pageable, 0xFF=not
	// disciple_inhibit_flag = 0;

	// Last OUT to port 0x1ffd: (byte 86)		// +3 and +2A only
	if (mmu->hasPort1ffd())
	{
		head.h2lenl	   = max(uint(head.h2lenl), z80v3len - 2u - z80v1len + 1u);
		head.port_1ffd = mmu->getPort1ffd();
	}

	// write header data to file:
	head.write(fd);


	/*	Now the compressed ram pages follow. Each block has a 3 byte header:
			dc.w		length of data (without this header; low byte first)
			dc.b		page number of block
			dc.s		compressed data follows

		The pages are numbered, depending on the hardware mode, in the following way:

			PageID	48 mode				128 mode			SamRam mode
			0		48K rom				rom (basic)			48K rom
			1		Interface I, Disciple or Plus D rom, according to setting
			2							rom (reset)			samram rom (basic)
			3		-					page 0				samram rom (monitor,..)
			4		8000-bfff			page 1				Normal 8000-bfff
			5		c000-ffff			page 2				Normal c000-ffff
			6		-					page 3				Shadow 8000-bfff
			7		-					page 4				Shadow c000-ffff
			8		4000-7fff			page 5				4000-7fff
			9		-					page 6				-
			10		-					page 7				-
			11		Multiface rom		Multiface rom		-
	*zxsp*	12		SPECTRA rom			SPECTRA rom			SPECTRA rom
	*zxsp*	13		SPECTRA ram[0]		SPECTRA ram[0]		SPECTRA ram[0]
	*zxsp*	14		SPECTRA ram[1]		SPECTRA ram[1]		SPECTRA ram[1]

		In 16k mode, only page 8 is saved.					kio 1999-05-02
		In 48k mode, pages 4,5 and 8 are saved.
		In SamRam mode, pages 4 to 8 must be saved.			((SamRam not supported))
		In 128 mode, all pages from 3 to 10 are saved.

		The 128 has a memory map like:   Rom [switchable];   Ram 5;   Ram 2;   Ram [switchable]
		*/


	// Custom ROM:
	// .z80 can only save 1 or 2 16k rom pages
	// either only one 16k rom page with id 0
	// or two rom pages: id 0 = basic rom, id 2 = boot rom
	// different rom sizes (zx80:4k, zx81:8k, ts2068:24k, +3/+2A:64k) cannot be saved!
	//
	//	TODO
	//

	// RAM:
	if (varying_ramsize)
	{
		uint32 addr	 = 0;
		uint   pages = head.spectator;

		for (int i = 0; i < 8; i++) // save pages with id 3..10 and size 1k..128k
		{
			if (pages & (1 << i))
			{
				write_compressed_page(fd, 3 + i, &ram[addr], 0x400 << i);
				addr += 0x400 << i;
			}
		}
	}
	else
	{
		uint n = ram.count() / 0x4000;
		if (n < 3) // no paging / no port 7ffd: write pages 8, 4 and 5:
		{
			write_compressed_page(fd, 8, &ram[0x0000], 0x4000);
			if (n > 1) { write_compressed_page(fd, 4, &ram[0x4000], 0x4000); }
			if (n > 2) { write_compressed_page(fd, 5, &ram[0x8000], 0x4000); }
		}
		else // paged memory		note: should work for 256k-Scorpion and Timex too
		{
			if (mmu->hasPort7ffd() && Mmu128kPtr(mmu)->port7ffdIsLocked())
			{
				// special service:
				// don't write unaccessible pages in 128/+2/+2A/+3:

				Mmu128k* mmu128 = Mmu128kPtr(mmu);

				assert(&ram[mmu128->getPage4000() << 14] == cpu->rdPtr(0x4000));
				assert(&ram[mmu128->getPage8000() << 14] == cpu->rdPtr(0x8000));
				assert(&ram[mmu128->getPageC000() << 14] == cpu->rdPtr(0xC000));

				uint32 visible = 1 << (mmu128->getVideopage()); // might be inaccessible

				visible |= 1 << (mmu128->getPage4000());
				visible |= 1 << (mmu128->getPage8000());
				visible |= 1 << (mmu128->getPageC000());

				if (mmu->hasPort1ffd() && MmuPlus3Ptr(mmu)->isRamOnlyMode())
				{
					assert(&ram[mmu128->getPage0000() << 14] == cpu->rdPtr(0x0000)); // DENK: ROMDIS?
					visible |= 1 << (mmu128->getPage0000());
				}

				CoreByte noram[0x4000];
				memset(&noram[0], 0, sizeof(CoreByte) * 0x4000);
				for (uint i = 0; i < n; i++)
					write_compressed_page(fd, i + 3, visible & (1 << i) ? &ram[i * 0x4000] : &noram[0], 0x4000);
			}
			else
			{
				for (uint i = 0; i < n; i++) write_compressed_page(fd, i + 3, &ram[i * 0x4000], 0x4000);
			}
		}
	}

	if (spectra)
	{
		if (spectra->isRomInserted()) write_compressed_page(fd, 12, &spectra->rom[0x0000], 0x4000);
		if (spectra->shadowram_ever_used) write_compressed_page(fd, 13, &spectra->shadowram[0x0000], 0x4000);
		if (spectra->shadowram_ever_used) write_compressed_page(fd, 14, &spectra->shadowram[0x4000], 0x4000);
	}
}


/*	attach joysticks:
	not shure about this - we have preferences for that
*/
void Machine::loadZ80_attach_joysticks(uint z80head_im)
{
	assert(isMainThread());

	Item* j	  = nullptr;
	int	  idx = 0;

	switch (z80head_im >> 6)
	{
	default: return;

	case 1:
		j = findIsaItem(isa_KempstonJoy);
		if (!j) j = addExternalItem(isa_KempstonJoy);
		break;

	case 3: // right (1st) IF2
		idx = 1;
		FALLTHROUGH
	case 2: // left (2nd) IF2
		j = findIsaItem(isa_SinclairJoy);
		if (!j) j = addExternalItem(isa_ZxIf2);
		break;
	}

	for (int i = 0; i < max_joy; i++)
	{
		if (joysticks[i] && joysticks[i]->isConnected())
		{
			static_cast<Joy*>(j)->insertJoystick(idx, i);
			break;
		}
	}
}


/*  load .z80 snapshot file
	the machine model must match the file!
	query model beforehand with Z80Head.getZxspModel()
	--> machine is powered up but suspended
*/
void Machine::loadZ80(FD& fd) noexcept(false) /*file_error,DataError*/
{
	xlogIn("Machine:loadZ80");

	assert(is_locked());

	bool ismainthread = isMainThread();

	Z80Head head;
	head.read(fd);

	// version 1.45 file

	if (head.isVersion145())
	{
		xlogline("Version 1.45");
		assert(ram.count() >= 0xc000);

		_suspend();
		_power_on();

		head.getRegisters(cpu->getRegisters());
		ula->setBorderColor(head.data >> 1);
		if (ismainthread) loadZ80_attach_joysticks(head.im);
		// else TODO .. ignore

		if (mmu->isA(isa_Mmu128k)) Mmu128kPtr(mmu)->setMmuLocked(true);

		if (head.data & 0x20) // compressed
		{
			xlogline("$c000 bytes compressed");
			read_compressed_page(fd, fd.file_size() - z80v1len, &ram[0], 0xc000);
		}
		else // uncompressed
		{
			xlogline("$c000 bytes uncompressed");
			read_uncompressed_page(fd, &ram[0], 0xc000);
		}
		return;
	}

	// version 2.01 or above

	xlogline(head.isVersion201() ? "Version 2.01" : "Version 3.00 or higher");

	if (head.isVersion201() && head.model >= 3 && head.model <= 4) head.model += 1; // z80 v2.01 -> v3.00

	// attach SPECTRA which has a Kempston joystick port:
	bool spectra_used	  = head.isVersion300() && (head.rldiremu & 0x08) && isA(isa_MachineZxsp); // else not supported
	SpectraVideo* spectra = findSpectraVideo();
	if (spectra_used && !spectra) spectra = addSpectraVideo(0); // configured later

	// attach joysticks:
	if (ismainthread) loadZ80_attach_joysticks(head.im);

	// attach IF1:
	bool if1_used = head.model == 1 || head.model == 5;
	if (if1_used)
	{
		if (head.if1paged) showAlert("ZX Interface-1 ROM paged in (TODO)");
		// else			  ignore: never used.
	}

	// attach MGT:
	bool mgt_used = head.model == 3 || head.model == 6;
	if (mgt_used)
	{
		if (head.mgt_paged) showAlert("M.G.T. ROM paged in: not supported");
		// else             ignore: never seen.
	}

	// attach AY sound chip:
	bool ay_used   = head.rldiremu & 0x04;
	bool fuller_ay = head.rldiremu & 0x40; // only if ay_used
	Ay*	 ay		   = this->ay;
	if (!ay) ay = AyPtr(findIsaItem(isa_Ay));
	if (ay_used && fuller_ay && (!ay || !ay->isA(isa_FullerBox))) ay = new FullerBox(this);
	if (ay_used && !ay) ay = new DidaktikMelodik(this);
	if (!this->ay) this->ay = ay;

	// attach external ram extension:
	uint32 req_ramsize = head.getRamsize(); // may be 0 for dflt ram size <=> no extension required
	if (req_ramsize == 0) req_ramsize = model_info->ram_size;
	if (ram.count() < req_ramsize)
	{
		Item* xram = findIsaItem(isa_ExternalRam);
		if (xram && !ismainthread)
			throw DataError("Cannot detach ram extension on background thread"); // can't happen for zxsp models
		else removeItem(xram);
		if (model == jupiter) addExternalRam(isa_Jupiter16kRam);
		else if (model_info->has_zxsp_bus) addExternalRam(isa_Cheetah32kRam);
		else if (model_info->has_zx80_bus)
		{
			if (req_ramsize <= 4 kB) new Zx3kRam(this, req_ramsize - 1 kB);
			else if (req_ramsize <= 16 kB) new Zx16kRam(this);
			else new Memotech64kRam(this);
		}
		if (ram.count() < req_ramsize)
			throw DataError("Snapshot: can't find a suitable ram extension to load this file");
	}

	// set T cycle counter and reset machine:
	int32 cc = head.isVersion300() ? head.getCpuCycle(model_info->cpu_cycles_per_frame) : 1000;

	_suspend();
	_power_on(cc);

	assert(current_cc() == cc);
	assert(now() == 0.0);

	// init cpu:
	head.getRegisters(cpu->getRegisters());

	// init ula:
	if (spectra_used)
	{
		uint bits = head.spectra_bits;
		spectra->enableNewVideoModes(bits & (1 << 0));
		spectra->setRS232Enabled(bits & (1 << 1));
		spectra->setJoystickEnabled(bits & (1 << 2));
		spectra->setIF1RomHooksEnabled(bits & (1 << 3));
		// if(bits&(1<<4)) spe->activateRom();			Später: erst Rom laden!
		if (spectra->rs232_enabled)
		{
			spectra->setPort239(0.0, ((bits >> 5) & 1) + ((bits >> 2) & 0x10) + 0b11101110);
			spectra->setPort247(0.0, (bits >> 7) & 1);
		}
		spectra->setBorderColor((head.data & 0xE0) | ((head.data >> 1) & 7));
	}

	ula->setBorderColor(head.data >> 1);

	// init mmu:
	mmu->setPort7ffd(head.port_7ffd);
	mmu->setPort1ffd(head.port_1ffd);
	mmu->setPortF4(head.port_f4);
	ula->setPortFF(head.port_ff); // mmu & ula
	if (fdc) fdc->initForSnapshot(cc);

	// init AY soundchip:
	if (ay)
	{
		ay->setRegisters(head.soundreg);
		ay->setRegNr(head.port_fffd);
	}

	// load memory pages:
	bool   varying_ramsize = head.varyingRamsize();
	uint   addr			   = 0;
	uint32 loaded		   = 0;
	bool   paged_mem	   = mmu->hasPort7ffd(); // for page numbering sceme

	while (!fd.is_at_eof())
	{
		uint len  = fd.read_uint16_z(); // compressed size
		uint page = fd.read_uint8();	// page

		xlogline("decompressing page %i", int(page));

		if (!paged_mem && !varying_ramsize && page == 8) page = 3; // convert page number 48k -> 128k

		if (page >= 3 && page < 32 + 3)
		{
			if (loaded & (1 << (page - 3))) throw DataError("Snapshot: page index occured twice");
			loaded |= 1 << (page - 3);
		}

		switch (page)
		{
		case 0:
			xlogline("--> Basic Rom"); // rom page 0: zxsp rom or +128k BASIC rom
			read_compressed_page(fd, len, &rom[0x0000], min(0x4000u, rom.count()));
			break;

		case 1:
			xlogline("--> IF1, Disciple or Plus D Rom: skipped!");
			fd.skip_bytes(len == 0xffff ? 0x4000 : len);
			break;

		case 2:
			xlogline("--> Boot Rom"); // rom page 1: +128k BOOT rom
			if (rom.count() < 0x8000) throw DataError("Snapshot: rom page index out of range");
			read_compressed_page(fd, len, &rom[0x4000], 0x4000);
			break;

		case 11:
			if (ram.count() > 128 kB) goto loadrampage; // Scorpion 256k etc.

			xlogline("--> Multiface Rom: skipped!");
			fd.skip_bytes(len == 0xffff ? 0x4000 : len);
			break;

		case 12:
			if (!spectra_used) goto loadrampage;

			xlogline("--> SPECTRA Rom Cartridge");
			spectra->filepath = newcopy("Snapshot page 12");
			spectra->rom	  = new Memory(this, "Snapshot page 12", 0x4000);
			read_compressed_page(fd, len, &spectra->rom[0], 0x4000);
			break;

		case 13:
		case 14:
			if (!spectra_used) goto loadrampage;

			xlogline("--> SPECTRA ram page #%i", page - 13);
			read_compressed_page(fd, len, &spectra->shadowram[(page - 13) * 0x4000], 0x4000);
			break;

		default:

		loadrampage:
			page -= 3; // load ram page page-3

			if (varying_ramsize)
			{
				if (page > 7) throw DataError("Snapshot: page index out of range");
				uint size = 0x400u << page;
				assert(addr + size <= ram.count());
				read_compressed_page(fd, len, &ram[addr], size);
				addr += size;
			}
			else
			{
				if (page >= (ram.count() >> 14)) throw DataError("Snapshot: page index out of range");
				read_compressed_page(fd, len, &ram[page << 14], 0x4000);
				if (spectra != nullptr)
				{
					if (mmu->hasPort7ffd())
					{
						if (page == 5) // also load to spectra_ram[0] if page 13 not yet loaded
							if (~loaded & (1 << 13)) cpu->c2c(&ram[5 << 14], &spectra->shadowram[0x0000], 0x4000);

						if (page == 7) // also load to spectra_ram[1] if page 14 not yet loaded
							if (~loaded & (1 << 14)) cpu->c2c(&ram[7 << 14], &spectra->shadowram[0x4000], 0x4000);
					}
					else if (page == 0) // also load to spectra_ram[0] if page 13 not yet loaded
						if (~loaded & (1 << 13)) cpu->c2c(&ram[0], &spectra->shadowram[0x0000], 0x4000);
				}
			}
			break;
		}
	}

	uint32 needed = varying_ramsize ? (req_ramsize / 0x400) // up to 255k
									  :
									  ~((0xffffffff << (req_ramsize / 0x4000))); // up to 512k
	if (needed & ~loaded) throw DataError("Snapshot: some pages are missing");

	if (spectra_used)
	{
		if (head.rldiremu & (1 << 4)) spectra->activateRom();
		spectra->setVideoMode(head.spectra_port_7fdf);
	}

	xlogline("loaded ok");
}
