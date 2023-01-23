// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	class PzxData
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












