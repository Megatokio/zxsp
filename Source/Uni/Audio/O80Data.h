#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "RlesData.h"
#include "TapeData.h"
#include "TapeFile.h"


/*	class O80Data
	-------------

	Subclass of "TapeData".

	A "O80Data" instance contains data from a .p, .o or similar file or converted to this format.
	A "O80Data" instance contains one data block.

	As all TapeData classes O80Data defines or implements conversion creators for CswBuffer:
	- new O80Data(CswBuffer const&) and
	- new CswBuffer(O80Data const&, uint32 ccps).

	And static file i/o methods:
	readFile (cstr fpath, TapeFile&) and writeFile (cstr fpath, TapeFile&)
*/


class O80Data : public TapeData
{
	friend CswBuffer::CswBuffer(const O80Data&, uint32);
	friend bool TapeFile::canBeSavedAs(cstr filename, cstr* why);

protected:
	Array<uint8> data;
	bool		 is_zx80;
	bool		 is_zx81;

private:
	void write_block_to_p81_file(FD&) const;
	void zx81_read_from_file(FD&, bool);

public:
	O80Data();
	O80Data(Array<uint8>, bool is_zx81);
	explicit O80Data(const CswBuffer&);
	explicit O80Data(const TapeData&);
	explicit O80Data(const O80Data&);
	explicit O80Data(const TzxData&); // in TzxData.cpp
	virtual ~O80Data();

	virtual cstr calcMajorBlockInfo() const noexcept; // NULL if n.avail.
	virtual cstr calcMinorBlockInfo() const noexcept; // NULL if n.avail.

	uint8*		 getData() { return data.getData(); }
	const uint8* getData() const { return data.getData(); }
	uint32		 count() const { return data.count(); }

	bool isZX80() { return is_zx80; }
	bool isZX81() { return is_zx81; }

	// read/write to file:
	static void readFile(cstr fpath, TapeFile&);
	static void writeFile(cstr fpath, TapeFile&);
};
