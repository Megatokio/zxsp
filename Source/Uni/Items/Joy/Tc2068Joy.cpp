// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Tc2068Joy.h"


namespace zxsp
{

Tc2068Joy::Tc2068Joy(Machine* m, isa_id id) : Joy(m, id, internal, nullptr, nullptr, "J1", "J2") {}
Ts2068Joy::Ts2068Joy(Machine* m) : Tc2068Joy(m, isa_Ts2068Joy) {}
U2086Joy::U2086Joy(Machine* m) : Tc2068Joy(m, isa_U2086Joy) {}

} // namespace zxsp
