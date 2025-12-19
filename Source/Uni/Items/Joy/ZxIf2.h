// Copyright (c) 2009 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Joy/SinclairJoy.h"
#include "Memory.h"


namespace zxsp
{

class ZxIf2 : public SinclairJoy
{
	MemoryPtr rom;
	cstr	  filepath;

public:
	explicit ZxIf2(Machine*);
	virtual ~ZxIf2() override;

	bool isLoaded() const { return rom.get() != nullptr; }
	cstr getFilepath() const { return filepath; }
	cstr getFilename() const { return basename_from_path(filepath); }

	void insertRom(cstr path);
	void ejectRom();

	// Item interface:
	void powerOn(/*t=0*/ int32 cc) override;
};

} // namespace zxsp
