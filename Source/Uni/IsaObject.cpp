// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "unix/FD.h"
#include "IsaObject.h"
#include "unix/FD.h"


/*	magic number for load/save:
*/
static const uint8 magic = 178;


/*	parent ids:
*/
isa_id isa_pid[] =
{
#define M_ISA(A,B,C) B
#include "isa_id.h"
};


/*	names:
*/
cstr isa_names[] =
{
#define M_ISA(A,B,C) C
#include "isa_id.h"
};



//virtual
void IsaObject::saveToFile( FD& fd ) const noexcept(false) /*file_error,bad_alloc*/
{
	fd.write_uint8(magic);
	fd.write_nstr(name);
}

//virtual
void IsaObject::loadFromFile( FD& fd ) noexcept(false) /*file_error,bad_alloc,data_error*/
{
	if(fd.read_uint8()!=magic) throw DataError("IsaObject magic corrupted");
	delete[]name; name=nullptr;
	name = fd.read_nstr();
}







