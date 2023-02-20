// Copyright (c) 2004 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Item.h"
#include "Inspector/Inspector.h"
#include "IoInfo.h"
#include "Machine.h"
#include "MachineController.h"
#include "TapeRecorder.h"
#include "globals.h"
#include <QEvent>
#include <QWidget>


// MachineController* Item::getMachineController() const
//{
//     return machine->controller;
// }


/*	convert Port Address Specs, e.g.: "----.----.--01.----"
	to Mask and Bits
*/
uint16 bitsForSpec(cstr s)
{
	for (uint16 bits = 0;;) switch (*s++)
		{
		case '1': bits += bits + 1; continue;
		case '0':
		case '-': bits += bits; continue;
		case '.': continue;
		case ' ': continue;
		case 0: return bits;
		default: IERR();
		}
}

uint16 maskForSpec(cstr s)
{
	for (uint16 mask = 0;;) switch (*s++)
		{
		case '0':
		case '1': mask += mask + 1; continue;
		case '-': mask += mask; continue;
		case '.': continue;
		case ' ': continue;
		case 0: return mask;
		default: IERR();
		}
}

cstr And(cstr a, cstr b)
{
	ptr z = dupstr(a);
	for (a = z; *z; z++, b++)
		if (*b != *z) *z = '-';
	return a;
}


void Item::unlink()
{
	if (machine && machine->lastitem == this) machine->lastitem = _prev;
	if (_prev) _prev->_next = _next;
	if (_next) _next->_prev = _prev;
	_prev = _next = 0;
}


void Item::linkBehind(Item* p)
{
	if (_prev || _next) unlink();
	_prev = p;
	_next = p ? p->_next : 0;
	if (_prev) _prev->_next = this;
	if (_next) _next->_prev = this;
	else machine->lastitem = this;
}


#define IOSZ 100

Item::Item(Machine* machine, isa_id id, isa_id grp, Internal internal, cstr o_addr, cstr i_addr) :
	IsaObject(id, grp),
	machine(machine),
	in_mask(0xffff), // dflt: no i/o
	in_bits(0),
	out_mask(0xffff),
	out_bits(0),
	_internal(internal),
	ioinfo(new IoInfo[IOSZ + 1]), // io info recorder
	ioinfo_count(0),
	ioinfo_size(IOSZ),
	ramdis_in(0),
	romdis_in(0)
{
	xlogIn("new Item: %s", name);

	// item list:
	Item* p = isA(isa_Ula) ? machine->cpu : machine->lastitem;
	_prev	= p;
	_next	= p ? p->_next : 0;
	if (_prev) _prev->_next = this;
	if (_next) _next->_prev = this;
	else machine->lastitem = this;

	// io masks:
	if (i_addr)
	{
		in_mask = maskForSpec(i_addr);
		in_bits = bitsForSpec(i_addr);
		xlogline("in:  mask=$%04x, bits=$%04x", int(in_mask), int(in_bits));
	}
	if (o_addr)
	{
		out_mask = maskForSpec(o_addr);
		out_bits = bitsForSpec(o_addr);
		xlogline("out: mask=$%04x, bits=$%04x", int(out_mask), int(out_bits));
	}

	machine->itemAdded(this);
}


Item::~Item()
{
	xlogIn("~Item: %s", name);

	unlink();
	machine->itemRemoved(this);
	delete[] ioinfo;
}


void Item::powerOn(int32)
{
	xlogline("Item:init: %s", name);

	// Items werden vom Specci nach hinten initialisert.
	// Weiter hinten angesteckte Items können also ggf.
	// in ihrem init() wieder prev()->romCS() aufrufen!
	romdis_in = 0;
	ramdis_in = 0;

	ioinfo_count = 0;
}


//bool Item::event(QEvent* e)
//{
//	xlogIn("Item[%s]:event: %s", name, QEventTypeStr(e->type()));
//	return QObject::event(e);
//	//	return 0;	// not processed
//}


/*	grow ioinfo[]
	adds one item more for the Renderers!
*/
void Item::grow_ioinfo()
{
	IoInfo* newioinfo = new IoInfo[2 * ioinfo_size + 1];
	memcpy(newioinfo, ioinfo, ioinfo_size * sizeof(IoInfo));
	delete[] ioinfo;
	ioinfo = newioinfo;
	ioinfo_size *= 2;
}


bool Item::is_locked() const volatile { return machine->is_locked(); }


void Item::lock() const volatile { machine->lock(); }

void Item::unlock() const volatile { machine->unlock(); }
