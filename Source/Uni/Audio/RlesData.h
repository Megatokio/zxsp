#pragma once
// Copyright (c) 2000 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

/*	class RlesData
	--------------

	run-length encoded 1-bit data block
	- read from rles file
	- recorded in the tape recorder when set to 1-bit mode

	can be converted directly from
	- mono data
	- stereo data
	- tzx or pzx
	- tap, o80 or p81

	can be played and recorded by the tape recorder

note:
	Framework	AudioToolbox/AudioToolbox.h
	Declared in	AudioToolbox/AudioFile.h
*/

#include "Templates/Array.h"
#include "CswBuffer.h"
#include "TapeFile.h"


class RlesData : public TapeData
{
public:
	RlesData();
	explicit RlesData(const TapeData&) noexcept(false); // data_error
	explicit RlesData(const TapData&)  noexcept(false); // data_error
	explicit RlesData(const TzxData&)  noexcept(false); // data_error
	explicit RlesData(const O80Data&)  noexcept(false); // data_error
	explicit RlesData(const RlesData&) noexcept(false); // data_error
	explicit RlesData(const AudioData&)noexcept(false); // data_error
	explicit RlesData(const CswBuffer&)noexcept(false); // data_error
	virtual	 ~RlesData();
	RlesData& operator=(const RlesData&) noexcept(false); // data_error

	static void readFile(cstr fpath, TapeFile&) throws;
	static void writeFile(cstr fpath, TapeFile&) throws;
};















