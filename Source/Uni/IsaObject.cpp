/*	Copyright  (c)	Günter Woigk 2004 - 2019
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
	if(fd.read_uint8()!=magic) throw data_error("IsaObject magic corrupted");
	delete[]name; name=NULL;
	name = fd.read_nstr();
}







