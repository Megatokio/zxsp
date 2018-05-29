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

#ifndef FDCPLUS3_H
#define FDCPLUS3_H

#include "Fdc765.h"


class FdcPlus3 : public Fdc765
{
public:
    explicit FdcPlus3(Machine*);

protected:
	// Item interface:
	//void	init			(/*t=0*/ int32 cc) override;
	//void	reset			(Time, int32 cc) override;
	void	input			(Time, int32 cc, uint16 addr, uint8& byte, uint8& mask) override;
	void	output			(Time, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time) override;
	//void	videoFrameEnd	(int32 cc) override;
	//void	saveToFile		(FD&) const throws override;
	//void	loadFromFile	(FD&) throws override;

	void	attachDiskDrive(uint n, FloppyDiskDrive*) override;
	void	removeDiskDrive(uint n) override;

private:
	bool	is_fault() override			{ return no; }
	bool	is_2sided() override		{ return drive->is_wprot; }
};


#endif
