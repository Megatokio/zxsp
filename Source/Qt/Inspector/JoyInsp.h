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
	int			 num_ports;
	QLineEdit*	 lineedit_display[3];
	uint8		 lineedit_state[3];
	QComboBox*	 joystick_selectors[3];
	QPushButton* button_scan_usb;
	QPushButton* button_set_keys;

public:
	JoyInsp(QWidget*, MachineController*, volatile IsaObject*, cstr img_path);

protected:
	void updateWidgets() override;

private:
	void update_joystick_selectors();
	void find_usb_joysticks();
	void set_keyboard_joystick_keys();
	void joystick_selected();
};

} // namespace gui
