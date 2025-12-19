// Copyright (c) 2002 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Screen.h"


namespace zxsp
{
struct IoInfo;

class ScreenZxsp : public Screen
{
	uint8*	_attrpixels;
	IoInfo* _ioinfo;
	uint	_ioinfo_count;
	uint	_cc_per_scanline;
	uint32	_cc_start_of_screenfile;
	uint32	_cc;
	bool	_flashphase;

protected:
	void do_ffb_or_vbi() override;

public:
	NO_COPY(ScreenZxsp);
	explicit ScreenZxsp(QWidget* owner, isa_id id = isa_ScreenZxsp) : Screen(owner, id) {}

	bool ffb_or_vbi(
		IoInfo* ioinfo, uint ioinfo_count, uint8* attr_pixels, uint32 cc_start_of_screenfile, uint cc_per_scanline,
		bool flashphase, uint32 cc) override;

	bool sendFrame(uint8*, const zxsp::Size&, const zxsp::Rect&) override { IERR(); } // b&w only
};

} // namespace zxsp


/*
























*/
