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

#include "Fdc.h"
#include "FloppyDiskDrive.h"



static FloppyDiskDrive* no_fdd = new FloppyDiskDrive();



Fdc::Fdc( Machine*m, isa_id id, Internal internal, cstr o_addr, cstr i_addr )
:
	MassStorage(m,id,internal,o_addr,i_addr),
	drive(no_fdd),
	motor_on(no),
	interrupt(no)
{
	assert(no_fdd);
	for(uint i=0; i<NELEM(fdd); i++) fdd[i] = no_fdd;
}


Fdc::~Fdc()
{
	for(uint i=0;i<NELEM(fdd);i++) removeDiskDrive(i);
}


void Fdc::powerOn(/*t=0*/ int32 cc)
{
	MassStorage::powerOn(cc);
	setMotor(0,off);
	interrupt=off;
}


void Fdc::reset( Time t, int32 cc)
{
	MassStorage::reset(t,cc);
	setMotor(t,off);
	interrupt=off;
}


void Fdc::attachDiskDrive(uint i, FloppyDiskDrive* dd)
{
	Fdc::removeDiskDrive(i);
	fdd[i] = dd;
	if(motor_on) dd->setMotor(0.0,on);
}


/*	remove drive.
	does not remove mirrored drives.
*/
void Fdc::removeDiskDrive(uint i)
{
	FloppyDiskDrive* dd = fdd[i];
	if(dd==no_fdd) return;
	fdd[i] = no_fdd;

	for(uint i=0;i<NELEM(fdd);i++) if(fdd[i]==dd) return;	// drive still exists at mirrored position

	if(drive==dd) drive = no_fdd;
	delete dd;
}


void Fdc::setMotor(Time t, bool f)
{
	if(motor_on!=f)
	{
		xlogline(f?"FDD: motor on":"FDD: motor off");
		for(uint i=0; i<NELEM(fdd); i++) fdd[i]->setMotor(t,f);
		motor_on = f;
	}
}


void Fdc::audioBufferEnd( Time t )
{
	for(uint j,i=0; i<NELEM(fdd); i++)
	{
		FloppyDiskDrive* dd = fdd[i];
		for(j=0; fdd[j]!=dd; j++) {}
		if(i==j) dd->audioBufferEnd(t);
	}
}











