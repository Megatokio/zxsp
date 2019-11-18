/*	Copyright  (c)	Günter Woigk 1994 - 2019
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

#ifndef TAPDATA_H
#define TAPDATA_H

#include "TapeData.h"
#include "RlesData.h"
#include "TapeFile.h"


/*	class TapData
	-------------

	Subclass of "TapeData".

	A "TapData" instance contains data from a .tap file or converted to .tap file format.
	A "TapData" instance contains one data block.

	As all TapeData classes TapData defines or implements conversion creators for CswBuffer:
	- new TapData(CswBuffer const&) and
	- new CswBuffer(TapData const&, uint32 ccps).

	And static file i/o methods:
	readFile (cstr fpath, TapeFile&) and writeFile (cstr fpath, TapeFile&)
*/


class TapData : public TapeData
{
public:
	Array<uint8>	data;
	bool			is_zxsp;			// we known for shure it's a ZX Spectrum tape block?
	bool			is_jupiter;			// we known for shure it's a Jupiter Ace tape block?
	Time			pause;				// silence after data
	uint			pilot_pulses;		// num. pilot pulses

	// informational data after conversion from CswBuffer:
	uint32			csw_pilot;			// start of pilot (index in csw[])
	uint32			csw_data;			// start of data
	uint32			csw_pause;			// start of pause

private:
	TapData&		operator=			(TapData const&);
					TapData				(TapData const&);

public:
	explicit		TapData				(bool is_zxsp, bool is_jupiter);
	explicit		TapData				(TapeData const&);
	explicit		TapData				(TzxData const&);		// in TzxData.cpp
	explicit		TapData				(CswBuffer const&);
					TapData				(uint16 const* csw, uint32 sz, uint32 ccps);
					TapData				(Array<uint8>&, uint ppilot, Time pause, bool is_zxsp);
	virtual			~TapData			();

	virtual cstr	calcMajorBlockInfo	() const noexcept;		// NULL if n.avail.
	virtual cstr	calcMinorBlockInfo	() const noexcept;		// NULL if n.avail.

	uint            count				() const                { return data.count(); }
	cu8ptr          getData             () const                { return data.getData(); }
	bool			isJupiter			() const				{ return is_jupiter; }
	bool			isZxSpectrum		() const				{ return is_zxsp; }

	void			writeToFile			(FD& fd, bool omit_typebyte) const throws;
	void			readFromFile		(FD& fd, bool add_typebyte) throws;

//static:
	static void		readFile			(cstr fpath, TapeFile&) throws;
	static void		writeFile			(cstr fpath, TapeFile&) throws;
};


#endif












