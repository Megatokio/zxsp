// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Zx3kInsp.h"
#include "Inspector.h"
#include "Items/Item.h"
#include "Items/Ram/Zx3kRam.h"
#include "Machine.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "Templates/NVPtr.h"
#include <QRadioButton>
#include <QRgb>
#include <QSettings>
#include <QVariant>


namespace gui
{

Zx3kInsp::Zx3kInsp(QWidget* parent, MachineController* mc, volatile Zx3kRam* zx3kram) :
	Inspector(parent, mc, zx3kram, "/Images/sinclair3k.jpg")
{
	button1k = new QRadioButton("1 kByte", this);
	button2k = new QRadioButton("2 kByte", this);
	button3k = new QRadioButton("3 kByte", this);

	button1k->move(8, 170);
	button2k->move(8, 190);
	button3k->move(8, 210);

	const QRgb fore = 0xffffffff;
	const QRgb back = 0;

	setColors(button1k, fore, back);
	setColors(button2k, fore, back);
	setColors(button3k, fore, back);

	uint size = zx3kram->getRamSize();
	button1k->setChecked(size == 1 kB);
	button2k->setChecked(size == 2 kB);
	button3k->setChecked(size == 3 kB);

	connect(button1k, &QRadioButton::clicked, this, [=] { set_ram_size(1 kB); });
	connect(button2k, &QRadioButton::clicked, this, [=] { set_ram_size(2 kB); });
	connect(button3k, &QRadioButton::clicked, this, [=] { set_ram_size(3 kB); });
}

void Zx3kInsp::set_ram_size(uint newsize)
{
	assert(machine);
	auto* zx3kram = dynamic_cast<volatile Zx3kRam*>(object);
	if (!zx3kram) return;

	if (newsize == zx3kram->getRamSize()) return;
	bool f = machine->powerOff();
	NV(zx3kram)->setRamSize(newsize);
	if (f) machine->powerOn();
	settings.setValue(key_zx3k_ramsize, newsize);
}

} // namespace gui
