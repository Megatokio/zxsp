/*	Copyright  (c)	Günter Woigk 2007 - 2018
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain_data the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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

























