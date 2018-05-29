/*	Copyright  (c)	Günter Woigk 2000 - 2012
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


	class PzxData
	--------------

	1-bit data block
	- read from pzx file

	may be convertible directly from
	- rles
	- tzx
	- tap, o80, p81  (eventually)
*/


#ifndef PzxData_h
#define PzxData_h

#include "TapeData.h"
#include "RlesData.h"
#include "MetaData.h"
#include "templates/Buffer.h"


typedef Buffer<uint8> charBuffer;


class PzxData : public TapeData
{
	charBuffer		data;
	mutable Time	current_position;
	Time			total_playtime;
	void			calcTotalPlaytime	();

	void			init				();
	void			init				( TapeData const& q );
	void			init				( PzxData const& q );
	void			init				( RlesData const& q );
	void			kill				();
	friend class RlesData;

public:				PzxData				()						:TapeData(isa_PzxData){ init(); }
					PzxData				( PzxData const& q )	:TapeData(q){ init(q); }	// shared data
					PzxData				( TapeData const& q )	:TapeData(q){ init(q); }	// convert
	PzxData&		operator=			( PzxData const& q )	{ if(this!=&q) { kill(); TapeData::operator=(q); init(q); } return *this; }
	PzxData&		operator=			( TapeData const& q )	{ if(this!=&q) { kill(); TapeData::operator=(q); init(q); } return *this; }
	PzxData&		operator=			( RlesData const& q )	{ kill(); TapeData::operator=(q); init(q); return *this; }
	virtual			~PzxData			()						{ kill(); }

	virtual bool	isA					( isa_id id ) const		{ return id==isa_PzxData || TapeData::isA(id); }
	virtual Time	getTotalPlaytime		() const				{ return total_playtime; }
	virtual Time	getCurrentPosition		() const				{ return current_position; }
	virtual void	calcBlockInfos		();
	virtual void	purge				();

	static void		readFile			( cstr fpath, AoP<TapeData>&, MetaData& ) throw(file_error,data_error,bad_alloc);
	static void		writeFile			( cstr fpath, AoP<TapeData>&, MetaData& ) throw(file_error,data_error,bad_alloc);
};

#endif












