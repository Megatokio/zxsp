#pragma once
// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Internal or external memory
	copy c'tor creates shared instances
*/

#include "Templates/Array.h"
#include "Templates/RCPtr.h"
#include "cpp/cppthreads.h"
#include "kio/kio.h"

using CoreByte = uint32; // Z80
class Machine;


class Memory
{
	friend class MemoryPtr;
	friend class RCPtr<Memory>;

	mutable uint _cnt;
	void		 retain() const { ++_cnt; }
	void		 release() const
	{
		if (--_cnt == 0) delete this;
	}

	void operator=(const Memory&); // prohibit
	Memory(const Memory&);		   // prohibit
	~Memory();					   // prohibit

public:
	// data members:
	Array<CoreByte> data;
	cstr			name; // e.g. "internal ram"
	Machine*		machine;


public:
	// c'tor:
	Memory(Machine*, cstr name, uint size) noexcept;

	// access data members:
	uint			count() const { return data.count(); }
	CoreByte*		getData() { return data.getData(); }
	const CoreByte* getData() const { return data.getData(); }
	cstr			getName() const { return name; }


	// get item at index:
	const CoreByte& operator[](uint i) const noexcept { return data[i]; }
	CoreByte&		operator[](uint i) noexcept { return data[i]; }

	// modifiy:
	void shrink(uint newcnt) noexcept;
	void grow(uint newcnt) noexcept;
};


class MemoryPtr : public RCPtr<Memory>
{
	MemoryPtr() {}

public:
	MemoryPtr(Memory* p) : RCPtr(p) {}
	~MemoryPtr() { assert(!p || p->_cnt > 0); }
	MemoryPtr(Machine* m, cstr name, uint cnt) : RCPtr(new Memory(m, name, cnt)) {}

	// access data members:
	uint			count() const { return p->data.count(); }
	CoreByte*		getData() { return p->data.getData(); }
	const CoreByte* getData() const { return p->data.getData(); }

	// get item at index:
	const CoreByte& operator[](uint i) const noexcept { return p->data[i]; }
	CoreByte&		operator[](uint i) noexcept { return p->data[i]; }

	// modifiy:
	void shrink(uint newcnt) { p->shrink(newcnt); }
	void grow(uint newcnt) { p->grow(newcnt); }
};
