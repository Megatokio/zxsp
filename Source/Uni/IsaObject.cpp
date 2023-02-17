// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "IsaObject.h"
#include "unix/FD.h"


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
