// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Printer.h"

namespace zxsp
{

Printer::Printer(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr) :
	Item(m, id, isa_Printer, internal, o_addr, i_addr)
{}

} // namespace zxsp
