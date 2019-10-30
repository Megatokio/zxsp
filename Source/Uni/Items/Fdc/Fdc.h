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

#ifndef FDC_H
#define FDC_H

#include "MassStorage.h"
class FloppyDiskDrive;


class Fdc : public MassStorage
{
protected:
	FloppyDiskDrive* fdd[4];	// attached drives, mirrored drives may be inserted more than once
	FloppyDiskDrive* drive;		// selected drive
	bool	motor_on;			// current state of the common motor-on output
	bool	interrupt;			// current state of the interrupt output; may be not connected

public:
	FloppyDiskDrive* getSelectedDrive() volatile const	{ return drive; }
	FloppyDiskDrive* getDrive(uint n) volatile const	{ return fdd[n&3]; }
	bool	isMotorOn() volatile const					{ return motor_on; }

VIR	void	setMotor(Time,bool);
VIR	void	initForSnapshot(int32 /*cc*/) {}
VIR	void	attachDiskDrive(uint n, FloppyDiskDrive*);
VIR	void	removeDiskDrive(uint n);

protected:
	Fdc(Machine*, isa_id, Internal internal, cstr o_addr, cstr i_addr);
	virtual ~Fdc();

	// Item interface:
	void	powerOn			(/*t=0*/ int32 cc) override;
	void	reset			(Time, int32 cc) override;
	//void	input			(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
	//void	output			(Time, int32 cc, uint16 addr, uint8 byte) override = 0;
	void	audioBufferEnd	(Time) override;
	//void	videoFrameEnd	(int32 cc);
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;
	//uint8	handleRomPatch	(uint16,uint8) override;

VIR	void	raise_interrupt()		{interrupt=on;}		// to be reimplemented if fdc uses interrupts
VIR	void	clear_interrupt()		{interrupt=off;}	// to be reimplemented if fdc uses interrupts
VIR	void	send_byte_to_dma(uint8)	{}					// to be reimplemented if fdc uses dma
VIR	uint8	read_byte_from_dma()	{return 0xFF;}		// to be reimplemented if fdc uses dma
};


#endif
