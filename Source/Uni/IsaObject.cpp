// Copyright (c) 2004 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "unix/FD.h"


namespace zxsp
{

/*	parent ids:
 */
isa_id isa_pid[] = {
#define M_ISA(A, B, C) B
#include "isa_id.h"
};


/*	names:
 */
cstr isa_names[] = {
#define M_ISA(A, B, C) C
#include "isa_id.h"
};

} // namespace zxsp
