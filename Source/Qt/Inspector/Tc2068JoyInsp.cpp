// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Tc2068JoyInsp.h"
#include "Ay/AySubclasses.h"
#include "Joy/Tc2068Joy.h"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>


namespace gui
{

Tc2068JoyInsp::Tc2068JoyInsp(QWidget* w, MachineController* mc, volatile IsaObject* j, cstr img_path) :
	JoyInsp(w, mc, j, img_path)
{
	assert(j->isA(isa_Tc2068Joy));

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10, 10, 10, 5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0, 100);
	g->setColumnStretch(0, 50);
	g->setColumnStretch(1, 50);

	g->addWidget(joystick_selectors[1], 1, 0);
	g->addWidget(joystick_selectors[0], 1, 1);
	g->addWidget(lineedit_display[1], 2, 0);
	g->addWidget(lineedit_display[0], 2, 1);

	g->addWidget(button_scan_usb, 3, 0, Qt::AlignHCenter | Qt::AlignVCenter);
	g->addWidget(button_set_keys, 3, 1, Qt::AlignHCenter | Qt::AlignVCenter);
}


void Tc2068JoyInsp::updateWidgets()
{
	xlogIn("Tc2068JoyInsp::updateWidgets");
	if (!object) return;

	for (int i = 0; i < num_ports; i++)
	{
		uint8 newstate = AyForTc2068::ayByteForJoystickByte(joy()->getStateForInspector(i));
		if (newstate == lineedit_state[i]) continue;
		lineedit_display[i]->setText(binstr(newstate, "%F000RLDU", "%--------"));
		lineedit_state[i] = newstate;
	}
}

} // namespace gui
