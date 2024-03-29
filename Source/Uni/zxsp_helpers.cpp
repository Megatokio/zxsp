// Copyright (c) 2002 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "zxsp_helpers.h"
#include "Files/RzxFile.h"
#include "Files/Z80Head.h"
#include "unix/files.h"
#include "zasm/Source/Z80Assembler.h"

constexpr uint snalen = 27;

bool ZxInfo::isA(isa_id i) const
{
	isa_id j = id;
	do {
		if (i == j) return yes;
	}
	while ((j = isa_pid[j]));
	return no;
}

void write_mem(FD& fd, const CoreByte* q, uint32 cnt)
{
	std::unique_ptr<uint8[]> bu {new uint8[cnt]};
	Z80::c2b(q, bu.get(), cnt);
	fd.write_bytes(bu.get(), cnt);
}

void read_mem(FD& fd, CoreByte* z, uint32 cnt)
{
	std::unique_ptr<uint8[]> bu {new uint8[cnt]};
	fd.read_bytes(bu.get(), cnt);
	Z80::b2c(bu.get(), z, cnt); // copy data, preserve flags
}

Model modelForSna(FD& fd)
{
	uint32 ramsize = uint32(fd.file_size()) - snalen;
	return ramsize > 0x4000 ? zxsp_i3 : zxsp_i1;
}

Model bestModelForFile(cstr fpath, Model default_model)
{
	if (!fpath) return default_model;
	Language language = zx_info[default_model].language;
	cstr	 ext	  = lowerstr(extension_from_path(fpath));

	if (eq(ext, ".tap") || eq(ext, ".tape"))
	{
		FD	   fd(fpath, 'r');
		uint16 bsize = fd.read_uint16_z();
		uint8  btype = fd.read_uint8();
		if ((bsize == 0x1a || bsize == 0x1b) && btype == 0) return jupiter;

		if (zx_info[default_model].isA(isa_MachineZxsp)) return default_model;
		goto dflt_zxsp;
	}

	if (eq(ext, ".sna"))
	{
		if (file_size(fpath) <= 0x4000 + snalen) return zxsp_i1;
		else goto dflt_zxsp;
	}

	if (eq(ext, ".z80"))
	{
		FD	  fd(fpath, 'r');
		Model m = modelForZ80(fd);
		return m == unknown_model ? default_model : m;
	}

	if (eq(ext, ".dsk")) { return language == spanish ? zxplus3_span : zxplus3; }

	if (eq(ext, ".o") || eq(ext, ".80")) { return zx80; }

	if (eq(ext, ".p") || eq(ext, ".81") || eq(ext, ".p81"))
	{
		if (zx_info[default_model].isA(isa_MachineZx81)) return default_model;
		else goto dflt_zx81;
	}

	if (eq(ext, ".ace")) { return jupiter; }

	if (eq(ext, ".scr"))
	{
		if (zx_info[default_model].isA(isa_MachineZxsp)) return default_model;
		else goto dflt_zxsp;
	}

	if (eq(ext, ".rom"))
	{
		uint32 fsz = uint32(file_size(fpath));
		if (zx_info[default_model].rom_size == fsz) return default_model;

		switch (fsz)
		{
		case 4 kB: return zx80; // currently original machine only, no clones
		case 8 kB:
			if (zx_info[default_model].isA(isa_MachineZx80))
				return default_model; // note: might be loaded into ZX80 as well
			else goto dflt_zx81;
		case 16 kB: goto dflt_zxsp;
		case 32 kB: return language == spanish ? zx128_span : zx128;
		case 64 kB: return language == spanish ? zxplus3_span : zxplus3;
		default: return default_model;
		}
	}

	if (eq(ext, ".hdf") || eq(ext, ".img") || eq(ext, ".dmg") || eq(ext, ".iso"))
	{
		if (zx_info[default_model].canAttachDivIDE()) return default_model;
		else return language == spanish ? zx128_span : zx128;
	}

	if (eq(ext, "rzx"))
	{
		fpath = RzxFile::getFirstSnapshot(fpath);
		return fpath ? bestModelForFile(fpath, default_model) : default_model;
	}

	if (eq(ext, ".src") || eq(ext, ".asm") || eq(ext, ".ass") || eq(ext, ".s"))
	{
		TempMemPool	 mempool;
		Z80Assembler ass;

		cstr destpath = catstr("/tmp/zxsp/s/", basename_from_path(fpath), ".$");
		ass.assembleFile(
			fpath,	  // source file must exist
			destpath, // destpath, dflt = source directory, may be dir or filename
			nullptr,  // listpath, dflt = dest direcory, may be dir or filename
			nullptr,  // temppath, dflt = dest dir, must be dir
			0,		  // liststyle: 0=none, 1=plain, 2=w/ocode, 4=w/labels, 8=w/clkcycles
			'b',	  // deststyle: 0=none, 'b'=binary
			no);	  // clean?

		if (ass.numErrors()) return default_model;
		else return bestModelForFile(ass.targetFilepath(), default_model);
	}

	// if(eq(ext,".tzx"))
	//{
	//	return default_model;		// all models can load from tape
	// }

	return default_model;

dflt_zxsp:
	return language == spanish	  ? inves :
		   language == portuguese ? tk95 : // language==american ? ts2068			TODO									
									zxsp_i3;

dflt_zx81:
	return language == american	  ? ts1000 :
		   language == portuguese ? tk85 : // language==spanish ? zx81
									zx81;
}
