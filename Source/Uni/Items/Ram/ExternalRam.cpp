// Copyright (c) 2012 - 2026 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ExternalRam.h"
#include "Item.h"


namespace zxsp
{

ExternalRam::ExternalRam(Machine* m, isa_id id) : Item(m, id, isa_ExternalRam, external, nullptr, nullptr) {}

} // namespace zxsp
