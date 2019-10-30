/*	Copyright  (c)	Günter Woigk 2014 - 2018
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

#ifndef DIVIDE_H
#define DIVIDE_H

#include "MassStorage.h"
#include "unix/files.h"
#include "IdeDevice.h"
#include "Uni/Memory.h"


class DivIDE : public MassStorage
{
	MemoryPtr	rom;
	MemoryPtr	ram;

	IdeDevice*	cf_card;

	uint8	ide_data_out_latch;		// 0 = write to latch, 1=write to IDE
	uint8	ide_data_in_latch;		// 0 = read from IDE,  1=read from latch
	bool	ide_data_latch_state;

	uint8	control_register;
	bool	jumper_E;
	bool	jumper_A;
	//bool	romdis_in;				// rear-side input state
	bool	auto_paged_in;			// auto page-in active?
	bool	own_romdis_state;		// own state = auto_paged_in + CONMEM bit
	cstr	romfilepath;

public:
	explicit DivIDE(Machine*);
	virtual ~DivIDE();

// ROM handling:
	cstr        getRomFilepath	() volatile const	{ return romfilepath; }
	cstr        getRomFilename	() volatile const	{ return basename_from_path(romfilepath); }
	bool		isRomPagedIn	() volatile const	{ return own_romdis_state; }
	int/*err*/	insertRom		(cstr path, bool silent=no);
	int/*err*/	insertDefaultRom(bool silent=false)	{ return insertRom(NULL,silent); }
	void		saveRom			(FD&) throws;

// Disk handling:
	bool		isDiskInserted	() volatile const	{ return cf_card != NULL; }
	void		insertDisk		(cstr path);
	void		ejectDisk		();

// Memory and Registers:
	void		setRamSize		(uint);
	MemoryPtr	getRam			()					{ return ram; }
	MemoryPtr	getRom			()					{ return rom; }
	void		setJumperE		(bool);				// 1 => rom paging enabled & eeprom write protected
	bool		getJumperE		() volatile const	{ return jumper_E; }
	uint8		getCtrlRegister	() volatile const	{ return control_register; }
	bool		getMapRam		() volatile const	{ return control_register & 0x40; }
	//bool		getIdeBusy		(Time t)			{ return cf_card && cf_card->isBusy(t); }

// for Inspector:
	bool	getIdeBusy		() volatile const { assert(isMainThread()); return cf_card && cf_card->is_busy(); }
	bool	isDiskWritable	() volatile const { assert(isMainThread()); return cf_card && cf_card->isWritable(); }
	cstr	getDiskFilename	() volatile const;
	Memory&	getRam			() volatile const { assert(isMainThread()); return const_cast<MemoryPtr&>(ram).ref(); }
	Memory&	getRom			() volatile const { assert(isMainThread()); return const_cast<MemoryPtr&>(rom).ref(); }
	//cstr	getDiskFilepath	() volatile const { assertMainThread(); return cf_card?cf_card->getFilepath():NULL; }
	void	setDiskWritable	(bool) volatile;

protected:
	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time, int32 cc) override;
	void	input			(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time, int32 cc, uint16 addr, uint8 byte) override;
	void	audioBufferEnd	(Time) override;
	//void	videoFrameEnd	(int32 cc);
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;
	uint8	handleRomPatch	(uint16,uint8) override;
	void	romCS			(bool f) override;

private:
	bool	mapram_is_set	()		{ return control_register & 0x40; }
	bool	conmem_is_set	()		{ return control_register & 0x80; }
	uint	mapped_rampage	()		{ return control_register & ((ram.count()>>13)-1); }
	void	mapMemory		();
	void	applyRomPatches	();
};


#endif


