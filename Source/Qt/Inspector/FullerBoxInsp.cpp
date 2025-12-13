// Copyright (c) 2009 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "FullerBoxInsp.h"
#include "Dialogs/ConfigureKeyboardJoystickDialog.h"
#include "UsbJoystick.h"
#include "zxsp_globals.h"
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>

namespace gui
{

FullerBoxInsp::FullerBoxInsp(QWidget* w, MachineController* mc, volatile FullerBox* i) :
	AyInsp(w, mc, i, "/Images/fuller_box.jpg"),
	fuller_box(i),
	js_state(0xff)
{
	layout->setContentsMargins(10, 10, 10, 10);

	js_display = new QLineEdit(this);
	js_display->setText("%--------");
	js_display->setAlignment(Qt::AlignHCenter);
	js_display->setReadOnly(true);

	js_selector = new QComboBox(this);
	js_selector->setFocusPolicy(Qt::NoFocus);
	update_joystick_selector();
	connect(
		js_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&FullerBoxInsp::slotJoystickSelected);

	button_scan_usb = new QPushButton("Scan USB", this);
	connect(button_scan_usb, &QPushButton::clicked, this, &FullerBoxInsp::slotFindUsbJoysticks);

	button_set_keys = new QPushButton("Set Keys", this);
	connect(button_set_keys, &QPushButton::clicked, this, &FullerBoxInsp::slotSetKeyboardJoystickKeys);

	layout->addWidget(js_display, 12, 2);
	layout->addWidget(js_selector, 13, 2);
	layout->addWidget(button_set_keys, 12, 3);
	layout->addWidget(button_scan_usb, 13, 3);
}

void FullerBoxInsp::updateWidgets()
{
	xlogIn("FullerBoxInsp::updateWidgets");
	assert(validReference(fuller_box));

	uint8 newstate = fuller_box->peekButtons();
	if (js_state != newstate)
	{
		js_state = newstate;
		js_display->setText(binstr(newstate, "F000RLDU", "--------"));
	}

	AyInsp::updateWidgets();
}

void FullerBoxInsp::update_joystick_selector()
{
	xlogIn("FullerBoxInsp::update_js_selector");
	assert(validReference(fuller_box));

	int num_needed = 2 + int(num_usb_joysticks);

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

	js_selector->setCurrentIndex(fuller_box->getJoystickID());
}

void FullerBoxInsp::slotJoystickSelected()
{
	xlogIn("FullerBoxInsp::slotJoystickSelected");
	assert(validReference(fuller_box));

	fuller_box->insertJoystick(JoystickID(js_selector->currentIndex()));
}

void FullerBoxInsp::slotFindUsbJoysticks()
{
	xlogIn("FullerBoxInsp::slotFindUsbJoysticks");

	findUsbJoysticks();
	update_joystick_selector();
}

void FullerBoxInsp::slotSetKeyboardJoystickKeys()
{
	xlogIn("FullerBoxInsp::slotSetKeyboardJoystickKeys");

	ConfigDialog* d = new ConfigureKeyboardJoystickDialog(controller);
	d->show();
}

} // namespace gui


/*




































*/
