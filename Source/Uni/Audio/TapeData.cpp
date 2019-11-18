/*	Copyright  (c)	Günter Woigk 2000 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.

	class TapeData
	base class for block data containers
	for use by class TapeFile
*/

#include "TapeData.h"
#include "globals.h"
#include "unix/files.h"


TapeData::TapeData( TapeData const& q )
:
	IsaObject(q),
	trust_level(q.trust_level)
{}


TapeData::TapeData(isa_id id , TrustLevel trustlevel)
:
	IsaObject(NULL,id,isa_TapeData),
	trust_level(trustlevel)
{}


TapeData::~TapeData()
{}


