// Copyright (c) 1995 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Crtc.h"

namespace zxsp
{
Crtc::Crtc(Machine* m, isa_id id, isa_id grp, Internal i, cstr o_addr, cstr i_addr) :
	Item(m, id, grp, i, o_addr, i_addr)
{}

} // namespace zxsp
