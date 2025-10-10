#pragma once
// Copyright (c) 2009 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Inspector.h"
class QLineEdit;
class QComboBox;
class QPushButton;
class QPushButton;


namespace gui
{

class JoyInsp : public Inspector
{
protected:
	volatile Joy* joy;

	uint		 num_ports;
	QLineEdit*	 lineedit_display[3];
	uint8		 lineedit_state[3];
	QComboBox*	 joystick_selectors[3];
	QPushButton* button_scan_usb;
	QPushButton* button_set_keys;

public:
	JoyInsp(QWidget*, MachineController*, volatile Joy*, cstr img_path);

protected:
	void		 updateWidgets() override;
	void		 update_joystick_selectors();
	virtual cstr lineedit_text(uint port, uint8 state);

private:
	void slotFindUsbJoysticks();
	void slotSetKeyboardJoystickKeys();
	void slotJoystickSelected();
};

} // namespace gui
