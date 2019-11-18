/*	Copyright  (c)	Günter Woigk 2012 - 2019
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


#include <QVariant>
#include <QSettings>
#include <QRgb>
#include <QRadioButton>
#include "Zx3kInsp.h"
#include "Qt/qt_util.h"
#include "Qt/Settings.h"
#include "Items/Item.h"
#include "Inspector.h"
#include "Items/Ram/Zx3kRam.h"
#include "Machine.h"
#include "Templates/NVPtr.h"


Zx3kInsp::Zx3kInsp(QWidget* parent, MachineController* mc, volatile IsaObject* item)
:
	Inspector(parent,mc,item,"/Images/sinclair3k.jpg")
{
	assert(object->isA(isa_Zx3kRam));

	button1k = new QRadioButton("1 kByte",this);
	button2k = new QRadioButton("2 kByte",this);
	button3k = new QRadioButton("3 kByte",this);

	button1k->move(8,170);
	button2k->move(8,190);
	button3k->move(8,210);

	const QRgb fore = 0xffffffff;
	const QRgb back = 0;

	setColors(button1k, fore, back);
	setColors(button2k, fore, back);
	setColors(button3k, fore, back);

	uint size = zx3kram()->getRamSize();
	button1k->setChecked(size==1 kB);
	button2k->setChecked(size==2 kB);
	button3k->setChecked(size==3 kB);

	connect(button1k, &QRadioButton::clicked, this, [=]{set_ram_size(1 kB);});
	connect(button2k, &QRadioButton::clicked, this, [=]{set_ram_size(2 kB);});
	connect(button3k, &QRadioButton::clicked, this, [=]{set_ram_size(3 kB);});
}

void Zx3kInsp::set_ram_size(uint newsize)
{
	if(newsize == zx3kram()->getRamSize()) return;
	machine->powerOff();
	NV(zx3kram())->setRamSize(newsize);
	machine->powerOn();
}




















