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


#ifndef CRTCTC2048_H
#define CRTCTC2048_H

#include "UlaZxsp.h"


class UlaTc2048 : public UlaZxsp
{
	uint8	byte_ff;

public:
	UlaTc2048(Machine*, isa_id);

	void	powerOn					(/*t=0*/ int32 cc) override;
	void	reset					(Time t, int32 cc) override;
	void	output					(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void	input					(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	int32	doFrameFlyback			(int32 cc) override;
	int32	updateScreenUpToCycle	(int32 cc) override;
	//void	drawVideoBeamIndicator	(int32 cc) override;
	void	markVideoRam			() override;
	//int32	addWaitCycles			(int32 cc, uint16 addr) volatile const override;	TODO ?

	uint8	getPortFE()				{ return ula_out_byte; }
	bool	is64ColumnMode()		{ return byte_ff&4; }

	bool	hasPortFF				() volatile const noexcept override	{ return yes; }
	void	setPortFF				(uint8) override;
	uint8   getPortFF				()	volatile const override			{ return byte_ff; }
};


#endif





























