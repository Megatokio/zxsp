// Copyright (c) 2015 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MassStorage.h"


MassStorage::MassStorage(Machine* m, isa_id id, Internal internal, cstr o_addr, cstr i_addr) :
	Item(m, id, isa_MassStorage, internal, o_addr, i_addr)
{}


MassStorage::~MassStorage() {}
