/*	Copyright  (c)	Günter Woigk 2004 - 2018
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







