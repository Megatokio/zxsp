/*	Copyright  (c)	Günter Woigk 2000 - 2018
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

#ifndef O80DATA_H
#define O80DATA_H

#include "TapeData.h"
#include "RlesData.h"
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
    friend CswBuffer::CswBuffer(O80Data const&,uint32);
	friend bool TapeFile::canBeSavedAs( cstr filename, cstr* why );

protected:
	Array<uint8>	data;
	bool			is_zx80;
	bool			is_zx81;

private:
	void	write_block_to_p81_file(FD&) const throws;
	void	zx81_read_from_file	(FD&, bool) throws;

public:
	O80Data();
	O80Data(Array<uint8>, bool is_zx81);
	explicit O80Data(const CswBuffer&);
	explicit O80Data(const TapeData&);
	explicit O80Data(const O80Data&);
	explicit O80Data(const TzxData&);		// in TzxData.cpp
	virtual ~O80Data();

	virtual cstr calcMajorBlockInfo() const noexcept;	// NULL if n.avail.
	virtual cstr calcMinorBlockInfo() const noexcept;	// NULL if n.avail.

	uint8*			getData()				{ return data.getData(); }
	uint8 const*	getData()		const	{ return data.getData(); }
	uint32			count()			const	{ return data.count(); }

	bool			isZX80()				{ return is_zx80; }
	bool			isZX81()				{ return is_zx81; }

	// read/write to file:
	static void	readFile(cstr fpath, TapeFile&) throws;
	static void	writeFile(cstr fpath, TapeFile&) throws;
};


#endif












