// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SinclairJoyInsp.h"
#include "Joy/SinclairJoy.h"
#include "Joystick.h"
#include <QGridLayout>
#include <QPushButton>
#include <QtGui>


namespace gui
{

/*		vstretch	reset
		joysel0		joysel1
		display0	display1
		scanusb		setkeys
*/


SinclairJoyInsp::SinclairJoyInsp(QWidget* w, MachineController* mc, volatile IsaObject* j, cstr img_path) :
	JoyInsp(w, mc, j, img_path)
{
	assert(object->isA(isa_SinclairJoy));

	if (j->isA(isa_ZxIf2)) return; // ZxIf2 macht sein eigenes Layout

	QGridLayout* g = new QGridLayout(this);
	g->setContentsMargins(10, 10, 10, 5);
	g->setVerticalSpacing(4);
	g->setRowStretch(0, 100);
	g->setColumnStretch(0, 50);
	g->setColumnStretch(1, 50);

	g->addWidget(joystick_selectors[0], 1, 0);
	g->addWidget(joystick_selectors[1], 1, 1);
	g->addWidget(lineedit_display[0], 2, 0);
	g->addWidget(lineedit_display[1], 2, 1);

	g->addWidget(button_scan_usb, 3, 0, Qt::AlignHCenter | Qt::AlignVCenter);
	g->addWidget(button_set_keys, 3, 1, Qt::AlignHCenter | Qt::AlignVCenter);
}


void SinclairJoyInsp::updateWidgets()		  // ZX Spectrum +2
{											  //	Sinclair 1: %---FUDRL active low oK
	xlogIn("SinclairJoyInsp::updateWidgets"); //	Sinclair 2: %---LRDUF active low oK

	if (!machine || !object) return;

	uint8 newstate = joy()->getStateForInspector(0); // Sinclair 1
	if (newstate != lineedit_state[0])
	{
		lineedit_state[0] = newstate = SinclairJoy::calcS1ForJoy(newstate);
		lineedit_display[0]->setText(binstr(newstate, "%000FUDRL", "%--------"));
	}

	newstate = joy()->getStateForInspector(1); // Sinclair 2
	if (newstate != lineedit_state[1])
	{
		lineedit_state[1] = newstate = SinclairJoy::calcS2ForJoy(newstate);
		lineedit_display[1]->setText(binstr(newstate, "%000LRDUF", "%--------"));
	}
}

} // namespace gui
