// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


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




















