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


	class RlesData
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

#ifndef RLESDATA_H
#define RLESDATA_H

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


#endif












