/*	Copyright  (c)	Günter Woigk 2006 - 2018
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

#ifndef JOY_H
#define JOY_H

#include "Item.h"
#include "Joystick.h"	// physical joysticks
class OverlayJoystick;


class Joy : public Item
{
	friend class AyForTc2068;

protected:
	Joystick*	joy[3];
	cstr		idf[3];
	OverlayJoystick* overlays[3];
	uint		num_ports;

public:
	virtual ~Joy();

	void	insertJoystick(int i, int id);
	void	insertJoystick(int id)						{ insertJoystick(0,id); }

	JoystickID	getJoystickID(int i=0) volatile const	{ return indexof(joy[i]); }
	const Joystick*	joystick(int i=0) const				{ return joy[i]; }
	volatile const Joystick* joystick(int i=0) volatile const { return joy[i]; }

	uint8	getStateForInspector(int i=0) volatile const{ return joy[i]->getState(no); }
	bool	isConnected(int i=0) volatile const			{ return joy[i]->isConnected(); }
	uint	getNumPorts() volatile const				{ return num_ports; }

protected:
	Joy(Machine*, isa_id, Internal, cstr o_addr, cstr i_addr, cstr idf1, cstr idf2=NULL, cstr idf3=NULL);

	// Item interface
	//void	powerOn			(/*t=0*/ int32 cc) override;
	//void	reset			(Time t, int32 cc) override;
	void	input			(Time t, int32 cc, uint16 addr, uint8& byte, uint8& mask) override = 0;
	//void	output			(Time t, int32 cc, uint16 addr, uint8 byte) override;
	//void	audioBufferEnd	(Time t) override;
	//void	videoFrameEnd	(int32 cc) override;
	void	saveToFile		(FD&) const throws override;
	void	loadFromFile	(FD&) throws override;
	//uint8	handleRomPatch	(uint16 pc, uint8 o) override;	// returns new opcode
	//void	triggerNmi		() override;
	//uint8	readMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
	//void	writeMemory		(Time t, int32 cc, uint16 addr, uint8 byte) override;  // for memory mapped i/o
};


#endif









