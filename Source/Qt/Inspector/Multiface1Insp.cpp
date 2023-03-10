// Copyright (c) 2012 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Multiface1Insp.h"
#include "MachineController.h"
#include "Multiface/Multiface1.h"
#include "OS/Joystick.h"
#include "Overlays/Overlay.h"
#include "Screen/Screen.h"
#include "Settings.h"
#include "Templates/NVPtr.h"
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
	connect(
		joystick_selector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
		&Multiface1Insp::slotJoystickSelected);
	update_joystick_selector();

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
	xlogIn("Multiface1Insp::scanUSB");
	findUsbJoysticks();
	update_joystick_selector();
}

void Multiface1Insp::slotJoystickSelected()
{
	xlogIn("Multiface1Insp::joystick_selected");
	assert(validReference(mf1));

	int j = joystick_selector->currentIndex();
	nvptr(mf1)->insertJoystick(JoystickID(joystick_selector->itemData(j).toInt()));
	controller->addOverlayJoy(nvptr(mf1));
}

void Multiface1Insp::update_joystick_selector()
{
	xlogIn("Multiface1Insp::update_joystick_selector");
	assert(validReference(mf1));

	char f[max_joy];
	int	 i;

	for (i = 0; i < max_joy; i++) { f[i] = joysticks[i]->isConnected() ? '1' : '0'; }

	for (i = 0; i < joystick_selector->count(); i++) { f[joystick_selector->itemData(i).toInt()] += 2; }

	for (i = 0; i < max_joy; i++)
	{
		if (f[i] != '0' && f[i] != '3') break;
	}
	if (i == max_joy) return; // no change

	static constexpr cstr jname[5] = {"USB Joystick 1", "USB Joystick 2", "USB Joystick 3", "Keyboard", "no Joystick"};

	joystick_selector->blockSignals(1);
	while (joystick_selector->count()) { joystick_selector->removeItem(0); }
	for (i = 0; i < max_joy; i++)
	{
		if (f[i] != '0') joystick_selector->addItem(jname[i], i);
	}
	joystick_selector->blockSignals(0);

	int id = mf1->getJoystickID();
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


/*

































*/
