/*	Copyright  (c)	Günter Woigk 2009 - 2019
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

#ifndef ZXIF2_H
#define ZXIF2_H

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


#endif
