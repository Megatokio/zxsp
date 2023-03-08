// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "JoyInsp.h"
#include "Dialogs/ConfigDialog.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "Joy/Joy.h"
#include "Joystick.h"
#include "MachineController.h"
#include "Templates/NVPtr.h"
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
		connect(
			joystick_selectors[i], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
			&JoyInsp::slotJoystickSelected);
	}

	update_joystick_selectors();

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
		int j = joystick_selectors[i]->currentIndex();
		nvptr(joy)->insertJoystick(i, JoystickID(joystick_selectors[i]->itemData(j).toInt()));
		controller->addOverlayJoy(nvptr(joy));
	}
}

void JoyInsp::updateWidgets() // Kempston
{
	xlogIn("JoyInsp::updateWidgets");
	assert(validReference(joy));

	for (uint i = 0; i < num_ports; i++)
	{
		uint8 newstate = NV(joy)->getButtonsFUDLR(i);
		if (newstate == lineedit_state[i]) continue;
		lineedit_state[i] = newstate;
		lineedit_display[i]->setText(binstr(newstate, "--------", "111FUDLR"));
	}
}


void JoyInsp::update_joystick_selectors()
{
	bool is_connected[max_joy];
	int	 i;
	bool is_in_list[max_joy] = {0, 0, 0, 0, 0};

	// find real-world joysticks:
	for (i = 0; i < max_joy; i++) { is_connected[i] = joysticks[i]->isConnected(); }

	// joysticks currently in selector list:
	for (i = 0; i < joystick_selectors[0]->count(); i++) is_in_list[joystick_selectors[0]->itemData(i).toInt()] = true;

	// compare:
	for (i = 0; i < max_joy; i++)
	{
		if (is_connected[i] != is_in_list[i]) break;
	}
	if (i == max_joy) return; // no change

	// first call or a joystick has been plugged / unplugged:

	static cstr jname[5] = {"USB Joystick 1", "USB Joystick 2", "USB Joystick 3", "Keyboard", "no Joystick"};

	// if selectors send events then slotSelectJoystick() will mess up the settings:
	for (uint s = 0; s < num_ports; s++) { joystick_selectors[s]->blockSignals(1); }

	// for all ports of this interface:
	for (uint s = 0; s < num_ports; s++)
	{
		// empty selector list:
		while (joystick_selectors[s]->count()) { joystick_selectors[s]->removeItem(0); }

		// add existing real-world joysticks to selector list
		// and select the currently selected one, default = no_joy:
		int selected_id	 = NV(joy)->getJoystickID(s); // id of the real-world joystick
		int selected_idx = -1;						  // index in list
		for (i = 0; i < max_joy; i++)
		{
			if (!is_connected[i]) continue;
			if (i == selected_id) selected_idx = joystick_selectors[s]->count();
			joystick_selectors[s]->addItem(jname[i], i);
		}
		joystick_selectors[s]->setCurrentIndex(selected_idx >= 0 ? selected_idx : joystick_selectors[s]->count() - 1);
	}

	for (uint s = 0; s < num_ports; s++) { joystick_selectors[s]->blockSignals(0); }
}

} // namespace gui
