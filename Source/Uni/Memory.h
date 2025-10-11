#pragma once
// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*
	Internal or external memory
	copy c'tor creates shared instances
*/

#include "Templates/Array.h"
#include "cpp/cppthreads.h"
#include "kio/kio.h"

using CoreByte = uint32; // Z80
class Machine;


class Memory
{
	friend class MemoryPtr;
	friend std::shared_ptr<Memory>;
	friend std::default_delete<Memory>;

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


class MemoryPtr : public std::shared_ptr<Memory>
{
	MemoryPtr() {}
	using super = std::shared_ptr<Memory>;

public:
	MemoryPtr(Memory* p) : super(p) {}
	~MemoryPtr() {}
	MemoryPtr(Machine* m, cstr name, uint cnt) : super(new Memory(m, name, cnt)) {}

	// access data members:
	uint			count() const { return get()->data.count(); }
	CoreByte*		getData() { return get()->data.getData(); }
	const CoreByte* getData() const { return get()->data.getData(); }

	// get item at index:
	const CoreByte& operator[](uint i) const noexcept { return get()->data[i]; }
	CoreByte&		operator[](uint i) noexcept { return get()->data[i]; }

	// modifiy:
	void shrink(uint newcnt) { get()->shrink(newcnt); }
	void grow(uint newcnt) { get()->grow(newcnt); }
};
