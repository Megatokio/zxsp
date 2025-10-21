// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface1Insp.h"
#include "MachineController.h"
#include "Multiface/Multiface1.h"
#include "Overlays/Overlay.h"
#include "Screen/Screen.h"
#include "Settings.h"
#include "UsbJoystick.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

namespace gui
{

Multiface1Insp::Multiface1Insp(QWidget* w, MachineController* mc, volatile Multiface1* mf) :
	MultifaceInsp(w, mc, mf, "Images/multiface1.jpg", QRect(224, 20, 30, 30)) // red button		x y w h
{
	chkbox_joystick_enabled = new QCheckBox("Joystick", this);
	connect(chkbox_joystick_enabled, &QCheckBox::clicked, this, &Multiface1Insp::slotEnableJoystick);

	joystick_selector = new QComboBox(this);
	joystick_selector->setFocusPolicy(Qt::NoFocus);
	update_joystick_selector();
	connect(
		joystick_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&Multiface1Insp::slotJoystickSelected);

	lineedit_display = new QLineEdit("%--------", this);
	lineedit_display->setAlignment(Qt::AlignHCenter);
	lineedit_display->setReadOnly(yes);
	lineedit_state = 0;

	button_scan_usb = new QPushButton("Scan USB", this);
	connect(button_scan_usb, &QPushButton::clicked, this, &Multiface1Insp::slotFindUsbJoysticks);


	chkbox_joystick_enabled->move(7, 191);

	button_scan_usb->move(3, 208);
	button_scan_usb->setFixedWidth(85);
	joystick_selector->move(92, 209);
	joystick_selector->setFixedWidth(122);
	lineedit_display->move(223, 212);
	lineedit_display->setFixedWidth(86);

	slotEnableJoystick(mf->isJoystickEnabled());
}

void Multiface1Insp::updateWidgets() // Kempston
{
	xlogIn("Multiface1Insp::updateWidgets");

	MultifaceInsp::updateWidgets();

	bool f = mf1->isJoystickEnabled();
	if (chkbox_joystick_enabled->isChecked() != f) slotEnableJoystick(f); // safety
	if (!f) return;														  // disabled

	uint8 newstate = mf1->peekJoystickButtonsFUDLR();
	if (lineedit_state == newstate) return;
	lineedit_state = newstate;
	lineedit_display->setText(binstr(newstate, "--------", "111FUDLR"));
}

void Multiface1Insp::slotEnableJoystick(bool f)
{
	settings.setValue(key_multiface1_enable_joystick, f);

	mf1->enableJoystick(f);
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

void Multiface1Insp::slotFindUsbJoysticks()
{
	xlogIn("Multiface1Insp::slotFindUsbJoysticks");
	findUsbJoysticks();
	update_joystick_selector();
}

void Multiface1Insp::slotJoystickSelected()
{
	xlogIn("Multiface1Insp::slotJoystickSelected");
	assert(validReference(mf1));

	mf1->insertJoystick(JoystickID(joystick_selector->currentIndex()));
}

void Multiface1Insp::update_joystick_selector()
{
	xlogIn("Multiface1Insp::update_joystick_selector");
	assert(validReference(mf1));

	int num_needed = 2 + int(num_usb_joysticks);

	while (joystick_selector->count() > num_needed) { joystick_selector->removeItem(num_needed); }

	for (int i = joystick_selector->count(); i < num_needed; i++)
	{
		if (i == 0) { joystick_selector->addItem("no Joystick"); }
		else if (i == 1) { joystick_selector->addItem("Keyboard"); }
		else
		{
			char idf[] = "USB Joystick #";
			idf[13]	   = char('0' + i - 2);
			joystick_selector->addItem(idf);
		}
	}

	joystick_selector->setCurrentIndex(mf1->getJoystickID());
}

} // namespace gui


/*

































*/
