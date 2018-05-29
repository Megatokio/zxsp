/*	Copyright  (c)	Günter Woigk 1995 - 2018
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

#ifndef ULA_H
#define ULA_H

#include "Item.h"
#include "Audio/StereoSample.h"
#include "Keymap.h"
#include "Crtc.h"


class Ula : public Crtc
{
protected:
	uint8	ula_out_byte;			// last out byte to ula: border, beeper, ear_out

	// Beeper/tape out:
	Sample	beeper_volume;			// 0.0 ... 1.0
	Sample	beeper_current_sample;	// current beeper elongation
	Time	beeper_last_sample_time;

public:
	Keymap	keymap;					// keyboard matrix as seen by ula

protected:
	Ula(Machine*, isa_id, cstr o_addr, cstr i_addr);

	uint8	readKeyboard(uint16 addr);	// read bits from keyboard matrix
VIR	void	setupTiming() = 0;

public:
// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	//void	reset			(Time t, int32 cc) override;
	//void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;

	Sample	getBeeperVolume	()					{ return beeper_volume; }
	void	setBeeperVolume	(Sample);

	void	setLinesBeforeScreen(int n);
	void	setLinesAfterScreen(int n);
	void	setCcPerLine	(int n);
	void	setBytesPerLine	(int n);

VIR uint8	getFloatingBusByte(int32 /*cc*/)	{ return 0xff; }
VIR int32	addWaitCycles(int32 cc, uint16 /*addr*/) volatile const { return cc; }
VIR int32	cpuCycleOfInterrupt() = 0;
VIR int32	cpuCycleOfIrptEnd() = 0;
VIR int32	cpuCycleOfFrameFlyback() = 0;		// when next ffb irpt
VIR uint8	interruptAtCycle(int32, uint16)		{ return 0xff; /*RST_38*/ }

VIR bool	hasPortFF() volatile const noexcept	{ return no; }
VIR void	setPortFF(uint8)					{}
VIR uint8   getPortFF()	volatile const			{ return 0xFF; }
};


#endif









