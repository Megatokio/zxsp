/*	Copyright  (c)	Günter Woigk 2004 - 2019
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
*/

#include <QWidget>
#include <QEvent>
#include "Inspector/Inspector.h"
#include "Item.h"
#include "Machine.h"
#include "MachineController.h"
#include "TapeRecorder.h"
#include "globals.h"
#include "IoInfo.h"


//MachineController* Item::getMachineController() const
//{
//    return machine->controller;
//}


/*	convert Port Address Specs, e.g.: "----.----.--01.----"
	to Mask and Bits
*/
uint16 bitsForSpec ( cstr s )
{
	for( uint16 bits=0;;)
		switch(*s++)
		{
		case '1': bits += bits +1; continue;
		case '0':
		case '-': bits += bits; continue;
		case '.': continue;
		case ' ': continue;
		case  0 : return bits;
		default: IERR();
		}
}

uint16 maskForSpec ( cstr s )
{
	for( uint16 mask=0;;)
		switch(*s++)
		{
		case '0':
		case '1': mask += mask +1; continue;
		case '-': mask += mask; continue;
		case '.': continue;
		case ' ': continue;
		case  0 : return mask;
		default: IERR();
		}
}

cstr And( cstr a, cstr b )
{
	ptr z = dupstr(a);
	for( a=z; *z; z++,b++ ) if(*b!=*z) *z='-';
	return a;
}


void Item::unlink()
{
	if(machine && machine->lastitem==this) machine->lastitem=_prev;
	if(_prev) _prev->_next=_next;
	if(_next) _next->_prev=_prev;
	_prev=_next=0;
}


void Item::linkBehind( Item* p )
{
	if(_prev||_next) unlink();
	_prev=p;
	_next=p?p->_next:0;
	if(_prev) _prev->_next=this;
	if(_next) _next->_prev=this;
	else machine->lastitem = this;
}


#define IOSZ 100

Item::Item ( Machine* machine, isa_id id, isa_id grp, Internal internal, cstr o_addr, cstr i_addr )
:	IsaObject(machine,id,grp),
	machine(machine),
	in_mask(0xffff),		// dflt: no i/o
	in_bits(0),
	out_mask(0xffff),
	out_bits(0),
	_internal(internal),
	ioinfo(new IoInfo[IOSZ+1]),	// io info recorder
	ioinfo_count(0),
	ioinfo_size(IOSZ),
	ramdis_in(0),
	romdis_in(0)
{
	xlogIn("new Item: %s",name);

// item list:
	Item* p = isA(isa_Ula) ? machine->cpu : machine->lastitem;
	_prev = p;
	_next = p ? p->_next : 0;
	if(_prev) _prev->_next = this;
	if(_next) _next->_prev = this;
	else machine->lastitem = this;

// io masks:
	if( i_addr )
	{
		in_mask = maskForSpec(i_addr);
		in_bits = bitsForSpec(i_addr);
		xlogline("in:  mask=$%04x, bits=$%04x", int(in_mask), int(in_bits));
	}
	if( o_addr )
	{
		out_mask = maskForSpec(o_addr);
		out_bits = bitsForSpec(o_addr);
		xlogline("out: mask=$%04x, bits=$%04x", int(out_mask), int(out_bits));
	}

	machine->itemAdded(this);
}


Item::~Item ( )
{
	xlogIn( "~Item: %s", name );

	unlink();
	machine->itemRemoved(this);
	delete[] ioinfo;
}


void Item::powerOn(int32)
{
	xlogline("Item:init: %s",name);

	// Items werden vom Specci nach hinten initialisert.
	// Weiter hinten angesteckte Items können also ggf.
	// in ihrem init() wieder prev()->romCS() aufrufen!
	romdis_in = 0;
	ramdis_in = 0;

	ioinfo_count = 0;
}


bool Item::event(QEvent*e)
{
	xlogIn("Item[%s]:event: %s",name,QEventTypeStr(e->type()));
	return QObject::event(e);
//	return 0;	// not processed
}



/*	grow ioinfo[]
	adds one item more for the Renderers!
*/
void Item::grow_ioinfo()
{
	IoInfo* newioinfo = new IoInfo[2*ioinfo_size+1];
	memcpy(newioinfo,ioinfo,ioinfo_size*sizeof(IoInfo));
	delete[] ioinfo;
	ioinfo = newioinfo;
	ioinfo_size *= 2;
}


bool Item::is_locked() volatile const
{
	return machine->is_locked();
}


void Item::lock() volatile const
{
	machine->lock();
}

void Item::unlock() volatile const
{
	machine->unlock();
}















