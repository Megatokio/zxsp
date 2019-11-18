/*	Copyright  (c)	Günter Woigk 2007 - 2019
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


	Internal or external memory
	copy c'tor creates shared instances
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "kio/kio.h"
#include "Templates/RCPtr.h"
#include "Templates/Array.h"
#include "cpp/cppthreads.h"

typedef uint32 CoreByte;	// Z80
class Machine;


class Memory
{
	friend class MemoryPtr;
	friend class RCPtr<Memory>;

	mutable uint _cnt;
	void		retain()  const				{ ++_cnt; }
	void		release() const				{ if(--_cnt==0) delete this; }

	void		operator=(Memory const&);	// prohibit
	Memory		(Memory const&);			// prohibit
	~Memory		();							// prohibit

// data members:
public:
	Array<CoreByte>	data;
	cstr		name;		// e.g. "internal ram"
	Machine*	machine;



public:

// c'tor:
	Memory(Machine*, cstr name, uint size)	noexcept;

// access data members:
	uint			count		() const			{ return data.count(); }
	CoreByte*		getData		()					{ return data.getData(); }
	CoreByte const*	getData		() const			{ return data.getData(); }
	cstr			getName		() const			{ return name; }


// get item at index:
	CoreByte const&	operator[]	(uint i) const		noexcept	{ return data[i]; }
	CoreByte&		operator[]	(uint i)			noexcept	{ return data[i]; }

// modifiy:
	void			shrink		(uint newcnt)		noexcept;
	void			grow		(uint newcnt)		noexcept;
};


class MemoryPtr : public RCPtr<Memory>
{
	MemoryPtr(){}

public:
	MemoryPtr		(Memory* p)      	:RCPtr(p)   {}
	//MemoryPtr		(RCPtr const& q)	:p(q.p) { retain(); }
	~MemoryPtr		()					{ assert(!p||p->_cnt>0); }
	MemoryPtr		(Machine* m, cstr name, uint cnt)	noexcept(false) /*limit_error*/	:RCPtr(new Memory(m,name,cnt)){}

// access data members:
	uint			count		() const			{ return p->data.count(); }
	CoreByte*		getData		()					{ return p->data.getData(); }
	CoreByte const*	getData		() const			{ return p->data.getData(); }
	CoreByte const*	getData		() volatile const	{ assert(isMainThread()); return p->data.getData(); }

	uint			count		() volatile const	{ assert(isMainThread()); return p->data.count(); }

// get item at index:
	CoreByte const&	operator[]	(uint i) volatile const	noexcept	{ assert(isMainThread()); return p->data[i]; }
	CoreByte const&	operator[]	(uint i) const		noexcept		{ return p->data[i]; }
	CoreByte&		operator[]	(uint i)			noexcept		{ return p->data[i]; }

// modifiy:
	void			shrink		(uint newcnt)						{ p->shrink(newcnt); }
	void			grow		(uint newcnt)		noexcept(false) /*limit_error*/	{ p->grow(newcnt); }
};



#endif

























