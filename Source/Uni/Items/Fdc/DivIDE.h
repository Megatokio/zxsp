#pragma once
/*	Copyright  (c)	Günter Woigk 2014 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

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





