/*	Copyright  (c)	Günter Woigk 2000 - 2018
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.


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












