// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include "Dialogs/ConfigDialog.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Joy/Joy.h"
#include "MachineController.h"
#include "Templates/NVPtr.h"
#include "UsbJoystick.h"
#include <QComboBox>
#include <QPushButton>
#include <QtGui>

namespace gui
{

JoyInsp::JoyInsp(QWidget* w, MachineController* mc, volatile Joy* joy, cstr imgpath) :
	Inspector(w, mc, joy, imgpath),
	joy(joy)
{
	num_ports = NV(joy)->getNumPorts();
	xlogIn("new JoyInsp for %s (%i ports)", object->name, num_ports);

	joystick_selectors[1] = joystick_selectors[2] = nullptr;
	lineedit_display[1] = lineedit_display[2] = nullptr;

	for (uint i = 0; i < num_ports; i++)
	{
		lineedit_display[i] = new QLineEdit(this);
		lineedit_display[i]->setText("%--------");
		lineedit_state[i] = 0;
		lineedit_display[i]->setAlignment(Qt::AlignHCenter);
		lineedit_display[i]->setReadOnly(yes);
		lineedit_display[i]->setMinimumWidth(100);
		joystick_selectors[i] = new QComboBox(this);
		joystick_selectors[i]->setFocusPolicy(Qt::NoFocus);
		joystick_selectors[i]->setMinimumWidth(80);
	}

	// NOTE: addItem() emits signal currentIndexChanged() which will call slotJoystickSelected()
	// therefore the signal is connected after update_joystick_selectors()
	update_joystick_selectors();

	for (uint i = 0; i < num_ports; i++)
		connect(
			joystick_selectors[i], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
			&JoyInsp::slotJoystickSelected);

	button_scan_usb = new QPushButton("Scan USB", this);
	button_scan_usb->setMinimumWidth(100);
	connect(button_scan_usb, &QPushButton::clicked, this, &JoyInsp::slotFindUsbJoysticks);

	button_set_keys = new QPushButton("Set Keys", this);
	button_set_keys->setMinimumWidth(100);
	connect(button_set_keys, &QPushButton::clicked, this, &JoyInsp::slotSetKeyboardJoystickKeys);

	timer->start(1000 / 10);
}


void JoyInsp::slotFindUsbJoysticks()
{
	xlogIn("JoyInsp::scanUSB");
	assert(validReference(joy));

	findUsbJoysticks();
	update_joystick_selectors();
}

void JoyInsp::slotSetKeyboardJoystickKeys()
{
	xlogIn("JoyInsp::setKeys");

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}

void JoyInsp::slotJoystickSelected()
{
	xlogIn("JoyInsp::joySelected");
	assert(validReference(joy));

	for (uint i = 0; i < num_ports; i++)
	{
		int idx = joystick_selectors[i]->currentIndex();
		if (idx >= 0) joy->insertJoystick(i, JoystickID(idx));
		if (idx >= 0) controller->addOverlayJoy(joy);
	}
}

void JoyInsp::updateWidgets() // Kempston
{
	xlogIn("JoyInsp::updateWidgets");
	assert(validReference(joy));

	for (uint i = 0; i < num_ports; i++)
	{
		if (joy->getJoystickID(i) != joystick_selectors[i]->currentIndex()) update_joystick_selectors();

		uint8 newstate = joy->peekButtonsFUDLR(i);
		if (newstate == lineedit_state[i]) continue;
		lineedit_state[i] = newstate;
		lineedit_display[i]->setText(lineedit_text(i, newstate));
	}
}

cstr JoyInsp::lineedit_text(uint __unused port, uint8 state)
{
	return binstr(state, "--------", "111FUDLR"); // Kempston
}

void JoyInsp::update_joystick_selectors()
{
	xlogIn("JoyInsp::update_js_selector");
	assert(validReference(joy));

	int num_needed = 2 + int(num_usb_joysticks);

	for (uint j = 0; j < num_ports; j++)
	{
		auto& js_selector = joystick_selectors[j];

		while (js_selector->count() > num_needed) { js_selector->removeItem(num_needed); }

		for (int i = js_selector->count(); i < num_needed; i++)
		{
			if (i == 0) { js_selector->addItem("no Joystick"); }
			else if (i == 1) { js_selector->addItem("Keyboard"); }
			else
			{
				char idf[] = "USB Joystick #";
				idf[13]	   = char('0' + i - 2);
				js_selector->addItem(idf);
			}
		}

		js_selector->setCurrentIndex(joy->getJoystickID(j));
	}
}

} // namespace gui
