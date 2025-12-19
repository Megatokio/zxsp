// Copyright (c) 2023 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "MGT.h"


namespace zxsp
{

static cstr o_addr = nullptr; // TODO
static cstr i_addr = nullptr; // TODO

MGT::MGT(Machine* m) : Fdc(m, isa_MGT, external, o_addr, i_addr) {}

} // namespace zxsp
