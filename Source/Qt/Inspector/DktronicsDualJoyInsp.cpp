// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "DktronicsDualJoyInsp.h"
#include "Joy/SinclairJoy.h"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>

namespace gui
{

DktronicsDualJoyInsp::DktronicsDualJoyInsp(QWidget* w, MachineController* mc, volatile DktronicsDualJoy* joy) :
	JoyInsp(w, mc, joy, "/Images/dktronics_dual_js_if.jpg")
{
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

cstr DktronicsDualJoyInsp::lineedit_text(uint port, uint8 state)
{
	// Port 1 = Kempston
	// Port 2 = Sinclair 2

	return port == 0 ? binstr(state, "--------", "111FUDLR") :
					   binstr(SinclairJoy::calcS2FromFUDLR(state), "%000LRDUF", "%--------");
}

/*
void DktronicsDualJoyInsp::updateWidgets()
{
	xlogIn("DktronicsDualJoyInsp::updateWidgets");
	assert(validReference(dkjoy));

	uint8 newstate = dkjoy->peekButtonsFUDLR(0); // Port 1 = Kempston
	if (newstate != lineedit_state[0])
	{
		lineedit_state[0] = newstate;
		lineedit_display[0]->setText(binstr(newstate, "--------", "111FUDLR"));
	}

	newstate = dkjoy->peekButtonsFUDLR(1); // Port 2 = Sinclair 2
	if (newstate != lineedit_state[1])
	{
		lineedit_state[1] = newstate;
		lineedit_display[1]->setText(binstr(SinclairJoy::calcS2FromFUDLR(newstate), "%000LRDUF", "%--------"));
	}
}
*/

} // namespace gui
