// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface1Insp.h"
#include "Machine.h"
#include "Multiface/Multiface1.h"
#include "Settings.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>


namespace gui
{

Multiface1Insp::Multiface1Insp(QWidget* w, MachineController* mc, volatile IsaObject* i) :
	MultifaceInsp(w, mc, i, "Images/multiface1.jpg", QRect(224, 20, 30, 30)) // red button		x y w h
{
	chkbox_joystick_enabled = new QCheckBox("Joystick", this);
	connect(chkbox_joystick_enabled, &QCheckBox::clicked, this, &Multiface1Insp::enable_joystick);

	joystick_selector = new QComboBox(this);
	joystick_selector->setFocusPolicy(Qt::NoFocus);
	connect(
		joystick_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&Multiface1Insp::joystick_selected);
	update_joystick_selector();

	lineedit_display = new QLineEdit("%--------", this);
	lineedit_display->setAlignment(Qt::AlignHCenter);
	lineedit_display->setReadOnly(yes);
	lineedit_state = 0;

	button_scan_usb = new QPushButton("Scan USB", this);
	connect(button_scan_usb, &QPushButton::clicked, this, &Multiface1Insp::find_usb_joysticks);


	chkbox_joystick_enabled->move(7, 191);

	button_scan_usb->move(3, 208);
	button_scan_usb->setFixedWidth(85);
	joystick_selector->move(92, 209);
	joystick_selector->setFixedWidth(122);
	lineedit_display->move(223, 212);
	lineedit_display->setFixedWidth(86);

	enable_joystick(multiface1()->joystick_enabled);
}


void Multiface1Insp::updateWidgets() // Kempston
{
	xlogIn("Multiface1Insp::updateWidgets");
	if (!machine || !object) return;

	MultifaceInsp::updateWidgets();

	bool f = multiface1()->joystick_enabled;
	if (chkbox_joystick_enabled->isChecked() != f) enable_joystick(f); // security only
	if (!f) return;													   // disabled

	uint8 newstate = multiface1()->joystick->getState(no);
	if (lineedit_state == newstate) return; // no change
	lineedit_state = newstate;

	char s[] = "%111FUDLR";
	for (int j = 0; j < 8; j++)
	{
		if (((~newstate) << j) & 0x80) s[j + 1] = '-';
	}

	lineedit_display->setText(s);
}


void Multiface1Insp::enable_joystick(bool f)
{
	settings.setValue(key_multiface1_enable_joystick, f);
	multiface1()->enableJoystick(f);
	chkbox_joystick_enabled->setChecked(f);
	button_scan_usb->setEnabled(f);
	joystick_selector->setEnabled(f);
	lineedit_display->setEnabled(f);
	if (!f)
	{
		lineedit_display->setText("%--------");
		lineedit_state = 0x00;
	}
}


void Multiface1Insp::find_usb_joysticks()
{
	xlogIn("Multiface1Insp::scanUSB");
	findUsbJoysticks();
	update_joystick_selector();
}


void Multiface1Insp::joystick_selected()
{
	xlogIn("Multiface1Insp::joySelected");
	int j = joystick_selector->currentIndex();
	multiface1()->insertJoystick(joystick_selector->itemData(j).toInt());
}


void Multiface1Insp::update_joystick_selector()
{
	char f[max_joy];
	int	 i;

	for (i = 0; i < max_joy; i++) { f[i] = joysticks[i]->isConnected() ? '1' : '0'; }

	for (i = 0; i < joystick_selector->count(); i++) { f[joystick_selector->itemData(i).toInt()] += 2; }

	for (i = 0; i < max_joy; i++)
	{
		if (f[i] != '0' && f[i] != '3') break;
	}
	if (i == max_joy) return; // no change

	static cstr jname[5] = {"USB Joystick 1", "USB Joystick 2", "USB Joystick 3", "Keyboard", "no Joystick"};

	joystick_selector->blockSignals(1);
	while (joystick_selector->count()) { joystick_selector->removeItem(0); }
	for (i = 0; i < max_joy; i++)
	{
		if (f[i] != '0') joystick_selector->addItem(jname[i], i);
	}
	joystick_selector->blockSignals(0);

	int id = multiface1()->getJoystickID();
	for (i = 0; i < joystick_selector->count(); i++)
	{
		if (joystick_selector->itemData(i).toInt() == id)
		{
			joystick_selector->setCurrentIndex(i);
			break;
		}
	}
	if (i == joystick_selector->count()) { joystick_selector->setCurrentIndex(i - 1); }
}

} // namespace gui
