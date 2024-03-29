// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "TapeData.h"

/*
	class TapeData
	base class for block data containers
	for use by class TapeFile
*/

TapeData::TapeData(const TapeData& q) : IsaObject(q), trust_level(q.trust_level) {}

TapeData::TapeData(isa_id id, TrustLevel trustlevel) : IsaObject(id, isa_TapeData), trust_level(trustlevel) {}

TapeData::~TapeData() {}
