#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeData.h"
#include "TapeFile.h"
#include "kio/kio.h"
class TzxBlock;


/*	class TzxData
	-------------

	Subclass of "TapeData".

	A "TzxData" instance contains data from a .tzx file or converted to .tzx file format.
	Typically a "TzxData" instance contains only one "TzxBlock",
	but blocks grouped with block 0x21 are stored in one "TzxData" block
	and blocks which do not end with a pause are concatenated as well.

	As all TapeData classes TzxData defines or implements conversion creators for CswBuffer:
	- new TzxData(CswBuffer const&) and
	- new CswBuffer(TzxData const&, uint32 ccps).

	And static file i/o methods:
	readFile (cstr fpath, TapeFile&) and writeFile (cstr fpath, TapeFile&)
*/


enum TzxConversionStyle {
	TzxConversionExact,	  // reproduce csw exactly
	TzxConversionDefault, // store O80 and Tap blocks compressed, store pause as pause
	TzxConversionIdealize // try to store idealized data of not recognized blocks
};


class TzxData : public TapeData
{
	friend CswBuffer::CswBuffer(const TzxData&, uint32);

	TzxBlock* data;

	TzxData(TzxBlock*, TrustLevel);

public:
	explicit TzxData(const TapData&);
	explicit TzxData(const O80Data&);
	explicit TzxData(const CswBuffer&, TzxConversionStyle);
	virtual ~TzxData();

	cstr getMajorBlockInfo() const noexcept; // NULL if n.avail.
	cstr getMinorBlockInfo() const noexcept; // NULL if n.avail.

	// static:
	static void readFile(cstr fpath, TapeFile&);
	static void writeFile(cstr fpath, TapeFile&, TzxConversionStyle);
};
