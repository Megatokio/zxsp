// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "InvesJoyInsp.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QtGui>


namespace gui
{

InvesJoyInsp::InvesJoyInsp(QWidget* w, MachineController* mc, volatile InvesJoy* joy) :
	JoyInsp(w, mc, joy, "/Images/inves_joy.jpg")
{
	QLabel* label = new QLabel("Buttons:");

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10, 10, 10, 5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0, 100);
	g->setColumnStretch(0, 25);
	g->setColumnStretch(1, 25);
	g->setColumnStretch(2, 50);

	g->addWidget(joystick_selectors[0], 1, 0, 1, 2);
	g->addWidget(button_set_keys, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);
	g->addWidget(label, 2, 0);
	g->addWidget(lineedit_display[0], 2, 1);
	g->addWidget(button_scan_usb, 2, 2, Qt::AlignHCenter | Qt::AlignVCenter);
}

} // namespace gui
