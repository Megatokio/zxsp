// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "SinclairJoyInsp.h"
#include "Joy/SinclairJoy.h"
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


SinclairJoyInsp::SinclairJoyInsp(QWidget* w, MachineController* mc, volatile SinclairJoy* joy, cstr img_path) :
	JoyInsp(w, mc, joy, img_path)
{
	if (joy->isA(isa_ZxIf2)) return; // subclass ZxIf2Insp uses a different layout

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

cstr SinclairJoyInsp::lineedit_text(uint port, uint8 state)
{
	// ZX Spectrum +2
	//	Sinclair 1: %---FUDRL active low oK
	//	Sinclair 2: %---LRDUF active low oK

	return port == 0 ? binstr(SinclairJoy::calcS1FromFUDLR(state), "%000FUDRL", "%--------") :
					   binstr(SinclairJoy::calcS2FromFUDLR(state), "%000LRDUF", "%--------");
}

/*
void SinclairJoyInsp::updateWidgets()
{
	// ZX Spectrum +2
	//	Sinclair 1: %---FUDRL active low oK
	//	Sinclair 2: %---LRDUF active low oK

	xlogIn("SinclairJoyInsp::updateWidgets");
	assert(validReference(joy));

	if (joy->getJoystickID(0) != joystick_selectors[0]->currentIndex()) update_joystick_selectors();
	if (joy->getJoystickID(1) != joystick_selectors[1]->currentIndex()) update_joystick_selectors();

	uint8 newstate = joy->peekButtonsFUDLR(0); // Sinclair 1
	if (newstate != lineedit_state[0])
	{
		lineedit_state[0] = newstate;
		lineedit_display[0]->setText(binstr(SinclairJoy::calcS1FromFUDLR(newstate), "%000FUDRL", "%--------"));
	}

	newstate = joy->peekButtonsFUDLR(1); // Sinclair 2
	if (newstate != lineedit_state[1])
	{
		lineedit_state[1] = newstate;
		lineedit_display[1]->setText(binstr(SinclairJoy::calcS2FromFUDLR(newstate), "%000LRDUF", "%--------"));
	}
}
*/

} // namespace gui
