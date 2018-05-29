/*	Copyright  (c)	Günter Woigk 2002 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "kio/kio.h"
#include "ZxInfo/ZxInfo.h"
#include "unix/files.h"
#include "Settings.h"
#include "Z80Head.h"
#include "RzxFile.h"
#include "unix/tempmem.h"
#include "zasm/Source/Z80Assembler.h"


Model bestModelForFile(cstr fpath)
{
	Model default_model = settings.get_Model(key_startup_model, zxsp_i3);
	if(!fpath) return default_model;
	Language language = zx_info[default_model].language;
	cstr ext = lowerstr(extension_from_path(fpath));

	if(eq(ext,".tap") || eq(ext,".tape"))
	{
		FD fd(fpath,'r');
		uint16 bsize = fd.read_uint16_z();
		uint8  btype = fd.read_uint8();
		if((bsize==0x1a || bsize==0x1b) && btype==0) return jupiter;

		if(zx_info[default_model].isA(isa_MachineZxsp)) return default_model;
		goto dflt_zxsp;
	}

	if(eq(ext,".sna"))
	{
		const uint snalen = 27;
		if(file_size(fpath)<=0x4000+snalen) return zxsp_i1;
		else goto dflt_zxsp;
	}

	if(eq(ext,".z80"))
	{
		FD fd(fpath,'r');
		Model m = modelForZ80(fd);
		return m == unknown_model ? default_model : m;
	}

	if(eq(ext,".dsk"))
	{
		return language == spanish ? zxplus3_span : zxplus3;
	}

	if(eq(ext,".o") || eq(ext,".80"))
	{
		return zx80;
	}

	if(eq(ext,".p") || eq(ext,".81") || eq(ext,".p81"))
	{
		if(zx_info[default_model].isA(isa_MachineZx81)) return default_model;
		else goto dflt_zx81;
	}

	if(eq(ext,".ace"))
	{
		return jupiter;
	}

	if(eq(ext,".scr"))
	{
		if(zx_info[default_model].isA(isa_MachineZxsp)) return default_model;
		else goto dflt_zxsp;
	}

	if(eq(ext,".rom"))
	{
		uint32 fsz = uint32(file_size(fpath));
		if(zx_info[default_model].rom_size==fsz) return default_model;

		switch(fsz)
		{
		case 4 kB:	return zx80;				// currently original machine only, no clones
		case 8 kB:	if(zx_info[default_model].isA(isa_MachineZx80))
						return default_model;	// note: might be loaded into ZX80 as well
					else goto dflt_zx81;
		case 16 kB:	goto dflt_zxsp;
		case 32 kB:	return language == spanish ? zx128_span : zx128;
		case 64 kB:	return language == spanish ? zxplus3_span : zxplus3;
		default:	return default_model;
		}
	}

	if(eq(ext,".hdf")||eq(ext,".img")||eq(ext,".dmg")||eq(ext,".iso"))
	{
		if(zx_info[default_model].canAttachDivIDE())
			return default_model;
		else return language==spanish ? zx128_span : zx128;
	}

	if(eq(ext,"rzx"))
	{
		fpath = RzxFile::getFirstSnapshot(fpath);
		return fpath ? bestModelForFile(fpath) : default_model;
	}

	if(eq(ext,".src") || eq(ext,".asm") || eq(ext,".ass") || eq(ext,".s"))
	{
		TempMemPool mempool;
		Z80Assembler ass;

		cstr destpath = catstr("/tmp/zxsp/s/",basename_from_path(fpath),".$");
		ass.assembleFile(
			fpath,		// source file must exist
			destpath,	// destpath, dflt = source directory, may be dir or filename
			NULL,		// listpath, dflt = dest direcory, may be dir or filename
			NULL,		// temppath, dflt = dest dir, must be dir
			0,			// liststyle: 0=none, 1=plain, 2=w/ocode, 4=w/labels, 8=w/clkcycles
			'b',		// deststyle: 0=none, 'b'=binary
			no);		// clean?

		if(ass.numErrors()) return default_model;
		else return bestModelForFile(ass.targetFilepath());
	}

	//if(eq(ext,".tzx"))
	//{
	//	return default_model;		// all models can load from tape
	//}

	return default_model;

dflt_zxsp:
	return language==spanish ? inves
		 : language==portuguese ? tk95
	//	 : language==american ? ts2068			TODO
		 : zxsp_i3;

dflt_zx81:
	return language==american ? ts1000
		 : language==portuguese ? tk85
	//	 : language==spanish ? zx81
		 : zx81;
}

