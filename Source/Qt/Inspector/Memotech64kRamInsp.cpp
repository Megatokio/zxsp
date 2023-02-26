// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Memotech64kRamInsp.h"
#include "Qt/Settings.h"
#include "Qt/qt_util.h"
#include "Templates/NVPtr.h"
#include <QComboBox>
#include <QLabel>


namespace gui
{

static constexpr uint dipsw[5] = {8, 4, 2, 6, 1}; // {0b1000,0b0100,0b0010,0b0110,0b0001};


Memotech64kRamInsp::Memotech64kRamInsp(QWidget* p, MachineController* mc, volatile Memotech64kRam* item) :
	Inspector(p, mc, item, "/Images/memopak64k.jpg")
{
	assert(object->isA(isa_Memotech64kRam));

	jumper = new QComboBox(this);
	jumper->move(10, 6);
	jumper->setFocusPolicy(Qt::NoFocus);

	jumper->insertItems(
		0, QStringList() << "1---:  all 64k Ram"
						 << "-1--:  8-12k: Ram"
						 << "--1-:  12-16k: Ram"
						 << "-11-:  8-16k: Ram"
						 << "---1:  8-16k: no Ram");

	switch (item->getDipSwitches()) // settings.value(key_memotech64k_dip_switches,6/*0b0110*/).toInt())
	{
	case /*0b1000*/ 8: jumper->setCurrentIndex(0); break;
	case /*0b0100*/ 4: jumper->setCurrentIndex(1); break;
	case /*0b0010*/ 2: jumper->setCurrentIndex(2); break;
	case /*0b0110*/ 6: jumper->setCurrentIndex(3); break;
	case /*0b0001*/ 1: jumper->setCurrentIndex(4); break;
	}

	connect(jumper, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int i) {
		assert(validReference(item));
		uint dip_switches = dipsw[i];
		nvptr(item)->setDipSwitches(dip_switches);
		settings.setValue(key_memotech64k_dip_switches, dip_switches);
	});

	QLabel* l = new QLabel("POKE 16388,255 : POKE 16389,255 : NEW", this);
	l->setFont(QFont("Lucida Grande", 11));
	setColors(l, 0xcccccc);
	l->move(10, 145);
}

} // namespace gui
