// Copyright (c) 2009 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "Ay/FullerBox.h"
#include "AyInsp.h"
class QComboBox;
class QPushButton;

namespace gui
{
class FullerBoxInsp : public AyInsp
{
public:
	FullerBoxInsp(QWidget*, MachineController*, volatile FullerBox*);

protected:
	//	void fillContextMenu(QMenu*) override;
	void updateWidgets() override;

private:
	void update_joystick_selector();
	void slotJoystickSelected();
	void slotFindUsbJoysticks();
	void slotSetKeyboardJoystickKeys();

	volatile FullerBox* fuller_box;
	QPushButton*		button_scan_usb;
	QPushButton*		button_set_keys;
	QLineEdit*			js_display;
	QComboBox*			js_selector;
	uint8				js_state;
};

} // namespace gui
