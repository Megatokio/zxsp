#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Joy/SinclairJoy.h"
#include "Memory.h"
#include "unix/files.h"


class ZxIf2 : public SinclairJoy
{
	MemoryPtr	rom;
	cstr        filepath;

public:
	explicit ZxIf2(Machine*);
	virtual ~ZxIf2();

	bool	isLoaded() volatile const		{ assert(isMainThread()); return rom.isnot(nullptr); }
	cstr	getFilepath() volatile const	{ return filepath; }
	cstr	getFilename() volatile const	{ return basename_from_path(filepath); }

	void	insertRom(cstr path);
	void	ejectRom();

// Item interface:
	void	powerOn(/*t=0*/ int32 cc) override;
};



