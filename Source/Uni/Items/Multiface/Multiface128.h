/*	Copyright  (c)	Günter Woigk 2012 - 2018
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
*/

#ifndef MULTIFACE128_H
#define MULTIFACE128_H

#include "Multiface.h"


class Multiface128 : public Multiface
{
	friend class Multiface128Insp;

	bool	mf_enabled;
	uint8	videopage;		// video page bit in port 7FFD

public:
    explicit Multiface128(Machine*);

protected:
	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;
	uint8	handleRomPatch	(uint16 pc, uint8 o) override;			// returns new opcode
	void	triggerNmi		() override;
	//uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	//void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};


#endif














