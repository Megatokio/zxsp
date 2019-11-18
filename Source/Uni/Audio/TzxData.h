/*	Copyright  (c)	Günter Woigk 2000 - 2019
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

#ifndef TZXDATA_H
#define TZXDATA_H

#include "kio/kio.h"
#include "TapeData.h"
#include "TapeFile.h"
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


enum TzxConversionStyle
{
	TzxConversionExact,		// reproduce csw exactly
	TzxConversionDefault,	// store O80 and Tap blocks compressed, store pause as pause
	TzxConversionIdealize	// try to store idealized data of not recognized blocks
};


class TzxData : public TapeData
{
	friend CswBuffer::CswBuffer(TzxData const&,uint32);

	TzxBlock* data;

	TzxData(TzxBlock*, TrustLevel);

public:
	explicit TzxData(TapData const&);
	explicit TzxData(O80Data const&);
	explicit TzxData(CswBuffer const&, TzxConversionStyle);
	virtual ~TzxData();

	cstr	getMajorBlockInfo() const noexcept;		// NULL if n.avail.
	cstr	getMinorBlockInfo() const noexcept;		// NULL if n.avail.

// static:
	static void readFile(cstr fpath, TapeFile&) throws;
	static void writeFile(cstr fpath, TapeFile&, TzxConversionStyle) throws;
};


#endif












